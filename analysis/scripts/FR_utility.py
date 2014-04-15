#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Thomas Keck 2014
# Structuring classes, which implement the logic of our functional approach for the FullReconstruction

import hashlib


class Resource(object):
    """
    Resources are provide values if certain requirements are fulfilled.
    To be more specific the Resource class is a generic Functor class, which provides a simple value under a given name.
    It's used to provide things like: Name of a particle, PDG code of a particle, the basf2 path object,...
    """
    def __init__(self, name, x, requires=None):
        """
        Creates a new Resource
            @param name the name of the resource. Other Actors can require this resource using its name.
            @param x the value of the resource. If another Actor requires this resource its value is passed to the Actor.
            @param requires these requirements have to be fulfilled before calling this Actor
        """
        ##  the name of the resource. Other Actors can require this resource using its name.
        self.name = name
        ## the value of the resource. If another Actor requires this resource its value is passed to the Actor.
        self.x = x
        ## these requirements have to be fulfilled before calling this Actor
        self.requires = [] if requires is None else requires

    def __call__(self, *args):
        """
        Returns the given value x under the given name
            @param args additional arguments are ignored
        """
        return {self.name: self.x}


class Function(object):
    """
    This Functor class wraps a normal function into an Actor!
    Imagine you have a function foo, which does some crazy stuff:
        def foo(path, particleList): ...
    You can use this class to add foo to the sequence of Actors in the FullReconstruction algorithm like this:
        seq.addFunction(foo, path='Path', particleList='K+')
    The first argument is always the wrapped functions, the following keyword arguments correspond to the arguments of your function,
    the value of the keyword arguments are the names of the requirements the system shall bin to this arguments, when the Functor is called.
    You can bind multiple requirements to one parameter of the function by passing a list of requirements as the corresponding keyword argument
        seq.addFunction(foo, path='Path', particleLists=['K+','pi-'])
    """
    def __init__(self, func, **kwargs):
        """
        Creates a new Actor
            @func The function which is called by this Actor
            @kwargs The requirements of the function. The given keyword arguments have to correspond to the arguments of the function!
        """
        ## the name of the actor, for the moment this is simply the name of the function itself
        self.name = func.__name__
        ## the function which is called by this actor
        self.func = func
        ## The kwargs provided by the user.
        self.parameters = kwargs
        ## These requirements have to be fulfilled before calling this Actor
        self.requires = []
        for (key, value) in self.parameters.iteritems():
            if isinstance(value, str):
                self.requires.append(value)
            else:
                self.requires += value

    def __call__(self, *arguments):
        """
        Calls the underlying function of this actor with the required arguments
            @arguments the argument which were required, in the same order as specified in self.requires
        """
        requirement_argument_mapping = dict(zip(self.requires, arguments))
        kwargs = {}
        for (key, value) in self.parameters.iteritems():
            if isinstance(value, str):
                kwargs[key] = requirement_argument_mapping[value]
            else:
                kwargs[key] = [requirement_argument_mapping[v] for v in value]
        result = self.func(**kwargs)
        return result


class Sequence(object):
    """
    The Sequence contains all the Actors (Functions and Resources).
    On run, the sequence solves the dependencies between the Actors
    and calls them in the correct order.
    """

    def __init__(self):
        """ Create a new Sequence """
        ## The Sequence which contains all Functions and Resources
        self.seq = []

    def addResource(self, *args, **kwargs):
        """ Appends a resource to the Sequence """
        self.seq.append(Resource(*args, **kwargs))

    def addFunction(self, *args, **kwargs):
        """ Appends a function to the Sequence """
        self.seq.append(Function(*args, **kwargs))

    def run(self, verbose):
        """
        Resolve dependencies of the Actors, by extracting step by step the Actors for which
        all their requirements are provided.
        Execute these Actors and store their results in results. These results are then used
        to provide the required arguments of the following resources.
        """
        results = dict()
        while len(self.seq) != 0:
            current = [item for item in self.seq if all(r in results for r in item.requires)]
            self.seq = [item for item in self.seq if not all(r in results for r in item.requires)]
            for item in current:
                arguments = [results[r] for r in item.requires]
                results.update(item(*arguments))
            if current == []:
                break

        if verbose:
            for x in self.seq:
                print x.name, 'needs', [r for r in x.requires if r not in results]


def createHash(*args):
    """
    Creates a unique has which depends on the given arguments
        @args arguments the hash depends on
    """
    return hashlib.sha1(str([str(v) for v in args])).hexdigest()
