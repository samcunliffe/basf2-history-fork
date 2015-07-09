from multiprocessing import Queue
from Queue import Empty


class Basf2CalculationQueue():

    """
    This class is a wrapper around a multiprocessing.Queue

    It can be used to send and receive values from the modules while processing the path.
    You can use it to save - for example - filepaths of outputfiles that you create on the fly.
    The added items are all of the type _Basf2CalculationQueueItem.
    The _Basf2CalculationQueue can be used as a dict. After the termination of the underlaying process
    you can access the different entries by their names you gave them when putting them on the queue.
    """

    def __init__(self):
        self.queue = Queue()
        self.results = dict()

    def put(self, name, item, **kwargs):
        """
        Put an item on the queue with the given name. Please keep that adding two items with the same name
        overrides one of them!
        """
        self.queue.put(Basf2CalculationQueueItem(name, item), **kwargs)

    def fill_results(self):
        """
        Fill the internal dict with the information of the queue.
        Do not call this but rather use the functions from _Basf2Calculation.
        Do only call this when the underlying process has ended.
        """
        while True:
            try:
                result = self.queue.get_nowait()
                self.results.update({result.name: result.item})
            except Empty:
                return

    def get(self, name):
        """
        Return the item with the given name or an Exception when it is not found.
        Do not call this but rather use the functions from _Basf2Calculation.
        """
        self.fill_results()
        return self.results[name]

    def get_keys(self):
        """
        Return all possible names of items saved in this queue.
        Do not call this but rather use the functions from _Basf2Calculation.
        """
        self.fill_results()
        return self.results.keys()


class Basf2CalculationQueueItem():

    """
    A placeholder for a tuple string, object.
    Do not create them by yourself.
    """

    def __init__(self, name, item):
        self.name = name
        self.item = item


class dotdict(dict):

    """dot.notation access to dictionary attributes"""

    def __getattr__(self, attr):
        return self.get(attr)
    __setattr__ = dict.__setitem__
    __delattr__ = dict.__delitem__


class ModuleStatistics():

    def __init__(self, stats, categories):
        self.name = stats.name
        self.time_sum = self.get_dict(stats.time_sum, categories)
        self.time_mean = self.get_dict(stats.time_mean, categories)
        self.time_stddev = self.get_dict(stats.time_stddev, categories)
        self.memory_sum = self.get_dict(stats.memory_sum, categories)
        self.memory_mean = self.get_dict(stats.memory_mean, categories)
        self.memory_stddev = self.get_dict(stats.memory_stddev, categories)
        self.calls = self.get_dict(stats.calls, categories)

    def get_dict(self, function, categories):
        return {name: function(category) for name, category in categories}


class Basf2CalculationQueueStatistics():

    """
    As the basf2 statistics is not pickable, wa can not store it into the result_queue.
    So we write a wrapper which unpacks the needed properties.
    """

    def __str__(self):
        return self.str

    def __repr__(self):
        return self.str

    def __init__(self, statistics):
        self.module = []

        categories = [("INIT", statistics.INIT),
                      ("BEGIN_RUN", statistics.BEGIN_RUN),
                      ("EVENT", statistics.EVENT),
                      ("END_RUN", statistics.END_RUN),
                      ("TERM", statistics.TERM),
                      ("TOTAL", statistics.TOTAL)]

        for stats in statistics.modules:
            self.append_module_statistics(stats, categories)

        self.append_module_statistics(statistics.getGlobal(), categories)

        self.str = statistics()

    def append_module_statistics(self, stats, categories):
        self.module.append(ModuleStatistics(stats, categories))
