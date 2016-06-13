#!/usr/bin/env python3
# -*- coding: utf-8 -*

import theano
import theano.tensor

import numpy
from collections import namedtuple


class State(object):

    def __init__(self, x=None, y=None, params=None, cost=None, updates=None, train_function=None, eval_function=None):
        self.x = x
        self.y = y

        # model parameters
        self.params = params
        self.cost = cost

        self.updates = updates

        self.train_function = train_function
        self.eval_function = eval_function


def get_model(number_of_features, number_of_events, parameters):

    x = theano.tensor.matrix('x')
    y = theano.tensor.vector('y', dtype='float32')

    # learning_rate = parameters.get('learning_rate', 0.1)
    learning_rate = 0.1

    n_in = number_of_features
    n_out = 1
    rng = numpy.random.RandomState(1234)
    w_values = numpy.asarray(
        rng.uniform(
            low=-numpy.sqrt(6. / (n_in + n_out)),
            high=numpy.sqrt(6. / (n_in + n_out)),
            size=(n_in, n_out)
        ),
        dtype=theano.config.floatX
    )

    w_values *= 4
    w = theano.shared(value=w_values, name='W', borrow=True)

    b_values = numpy.zeros((n_out,), dtype=theano.config.floatX)
    b = theano.shared(value=b_values, name='b', borrow=True)

    activation = theano.tensor.nnet.sigmoid

    output = activation(theano.tensor.dot(x, w) + b)

    cost = theano.tensor.nnet.binary_crossentropy(output.T, y).mean()

    params = [w, b]

    grad_params = [theano.tensor.grad(cost, param) for param in params]

    updates = [(param, param - learning_rate * gparam) for param, gparam in zip(params, grad_params)]

    train_function = theano.function(
        inputs=[x, y],
        outputs=cost,
        updates=updates
    )

    eval_function = theano.function(
        inputs=[x],
        outputs=output
    )

    return State(x, y, params, cost, updates, train_function, eval_function)


def load(obj):
    return obj


def apply(state, X):
    result = state.eval_function(X)
    return result


def begin_fit(state):
    return state


def partial_fit(state, X, y, w, Xvalid, yvalid, wvalid, epoch):
    avg_cost = state.train_function(X, y) / len(y)
    print("Epoch:", '%04d' % (epoch), "cost=", "{:.9f}".format(avg_cost))
    return True


def end_fit(state):
    # FIXME: this won't work
    return state
