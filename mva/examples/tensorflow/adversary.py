#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2017

import numpy as np
import tensorflow as tf
import basf2_mva

from basf2_mva_python_interface.tensorflow import State


def get_variables(variables):
    result = [tv for v in variables for tv in tf.all_variables() if v == tv.name]
    if len(result) != len(variables):
        print([tv.name for tv in tf.all_variables()])
        print([v for v in variables if v not in list(tf.all_variables())])
        raise RuntimeError("Did not find all variables in tensorflow scope")
    return result


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):

    tf.reset_default_graph()
    x = tf.placeholder(tf.float32, [None, number_of_features])
    y = tf.placeholder(tf.float32, [None, 1])
    z = tf.placeholder(tf.float32, [None, number_of_spectators])
    w = tf.placeholder(tf.float32, [None, 1])

    def layer(x, shape, name, unit=tf.sigmoid):
        with tf.name_scope(name) as scope:
            weights = tf.Variable(tf.truncated_normal(shape, stddev=1.0 / np.sqrt(float(shape[0]))), name='weights')
            biases = tf.Variable(tf.constant(0.0, shape=[shape[1]]), name='biases')
            layer = unit(tf.matmul(x, weights) + biases)
        return layer

    inference_hidden1 = layer(x, [number_of_features, 20], 'inference_hidden1')
    inference_hidden2 = layer(inference_hidden1, [20, 20], 'inference_hidden2')
    inference_hidden3 = layer(inference_hidden2, [20, 20], 'inference_hidden3')
    inference_hidden4 = layer(inference_hidden3, [20, 20], 'inference_hidden4')
    inference_activation = layer(inference_hidden4, [20, 1], 'inference_sigmoid', unit=tf.sigmoid)

    epsilon = 1e-5
    inference_loss = -tf.reduce_sum(y * w * tf.log(inference_activation + epsilon) +
                                    (1.0 - y) * w * tf.log(1 - inference_activation + epsilon)) / tf.reduce_sum(w)
    inference_loss = tf.reduce_sum((y-inference_activation)*(y-inference_activation))
    for i in range(number_of_spectators):
        z_single = tf.slice(z, [0, i], [-1, 1])
        adversary_hidden1 = layer(inference_activation, [1, 20], 'adversary_hidden1_{}'.format(i), unit=tf.tanh)
        adversary_hidden2 = layer(adversary_hidden1, [20, 20], 'adversary_hidden2_{}'.format(i), unit=tf.nn.relu)
        adversary_means = layer(adversary_hidden2, [20, 4], 'adversary_means_{}'.format(i), unit=tf.identity)
        adversary_widths = layer(adversary_hidden2, [20, 4], 'adversary_width_{}'.format(i), unit=tf.exp)
        adversary_fractions_not_normed = layer(adversary_hidden2, [20, 4], 'adversary_fractions_{}'.format(i), unit=tf.identity)
        adversary_fractions = tf.nn.softmax(adversary_fractions_not_normed)
        adversary_activation = tf.reduce_sum(adversary_fractions *
                                             tf.exp(-(adversary_means - z_single) * (adversary_means - z_single) /
                                                    (2 * adversary_widths)))
        adversary_loss = -tf.reduce_sum(w * tf.log(adversary_activation + epsilon)) / tf.reduce_sum(w)
        tf.add_to_collection('adversary_losses', adversary_loss)

    if number_of_spectators > 0:
        adversary_loss = tf.add_n(tf.get_collection('adversary_losses'), name='adversary_loss')
        total_loss = inference_loss - parameters.get('lambda', 50) * adversary_loss
    else:
        adversary_loss = None
        total_loss = inference_loss

    adversary_vars = [
        'adversary_hidden1_{}/weights:0',
        'adversary_hidden1_{}/biases:0',
        'adversary_hidden2_{}/weights:0',
        'adversary_hidden2_{}/biases:0',
        'adversary_means_{}/weights:0',
        'adversary_width_{}/biases:0',
        'adversary_fractions_{}/weights:0',
        'adversary_fractions_{}/biases:0',
    ]
    adversary_vars = [v.format(i) for i in range(number_of_spectators) for v in adversary_vars]

    if number_of_spectators > 0:
        adversary_optimizer = tf.train.AdamOptimizer(learning_rate=parameters.get('learning_rate', 0.001))
        adversary_global_step = tf.Variable(0, name='adversary_global_step', trainable=False)
        adversary_minimize = adversary_optimizer.minimize(adversary_loss, global_step=adversary_global_step,
                                                          var_list=get_variables(adversary_vars))
    else:
        adversary_minimize = None

    inference_vars = [
        'inference_hidden1/weights:0',
        'inference_hidden1/biases:0',
        'inference_hidden2/weights:0',
        'inference_hidden2/biases:0',
        'inference_hidden3/weights:0',
        'inference_hidden3/biases:0',
        'inference_hidden4/weights:0',
        'inference_hidden4/biases:0',
        'inference_sigmoid/weights:0',
        'inference_sigmoid/biases:0',
    ]

    inference_optimizer = tf.train.AdamOptimizer(learning_rate=parameters.get('learning_rate', 0.001))
    inference_global_step = tf.Variable(0, name='inference_global_step', trainable=False)
    inference_minimize = inference_optimizer.minimize(total_loss, global_step=inference_global_step,
                                                      var_list=get_variables(inference_vars))

    init = tf.global_variables_initializer()

    config = tf.ConfigProto()
    config.gpu_options.allow_growth = True
    session = tf.Session(config=config)

    session.run(init)
    state = State(x, y, inference_activation, total_loss, inference_minimize, session)
    state.adversary_cost = adversary_loss
    state.adversary_optimizer = adversary_minimize
    state.w = w
    state.K = parameters.get('adversary_steps', 10)
    state.z = z

    return state


def partial_fit(state, X, S, y, w, epoch):
    """
    Pass received data to tensorflow session
    """
    feed_dict = {state.x: X, state.y: y, state.w: w, state.z: S}
    if epoch % state.K == 0 or state.adversary_optimizer is None:
        state.session.run(state.optimizer, feed_dict=feed_dict)
        avg_cost = state.session.run(state.cost, feed_dict=feed_dict)
        if epoch % 100 == 0:
            print("Epoch:", '%04d' % (epoch), "cost=", "{:.9f}".format(avg_cost))
    else:
        state.session.run(state.adversary_optimizer, feed_dict=feed_dict)
    return True


if __name__ == "__main__":

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector("train.root")
    general_options.m_treename = "tree"
    variables = ['p', 'pz', 'daughter(0, p)', 'daughter(0, pz)', 'daughter(1, p)', 'daughter(1, pz)',
                 'chiProb', 'dr', 'dz', 'daughter(0, dr)', 'daughter(1, dr)', 'daughter(0, chiProb)', 'daughter(1, chiProb)',
                 'daughter(0, Kid)', 'daughter(0, piid)', 'daughterAngle(0, 1)']
    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_spectators = basf2_mva.vector('M')
    general_options.m_target_variable = "isSignal"

    specific_options = basf2_mva.PythonOptions()
    specific_options.m_framework = "tensorflow"
    specific_options.m_steering_file = 'mva/examples/tensorflow_adversary.py'
    specific_options.m_nIterations = 400
    specific_options.m_mini_batch_size = 100

    for i, l in enumerate([1, 2, 5, 10, 20, 50, 100]):
        general_options.m_identifier = "TensorflowAdversary_{}".format(i)
        specific_options.m_config = '{"adversary_steps": 20, "learning_rate": 0.001, "lambda": ' + str(i) + '}'
        basf2_mva.teacher(general_options, specific_options)
