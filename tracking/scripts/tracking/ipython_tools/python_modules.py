# basf2 imports
import basf2
import ROOT
from ROOT import Belle2


class PrintCollections(basf2.Module):

    """
    A small module to catch the content of the store array for some events and store them in the queue.
    """

    def __init__(self, queue):
        """
        Create the module with the given queue.
        """
        self.queue = queue
        basf2.Module.__init__(self)
        self.store_content_list = []
        self.event_number = 0
        self.total_number_of_events = 0

    def initialize(self):
        """
        Get the total number of events from C++
        """
        import ROOT
        self.total_number_of_events = ROOT.Belle2.Environment.Instance().getNumberOfEvents()

    def event(self):
        """
        Write the store array content into a list for later.
        """
        if self.total_number_of_events == 0:
            return

        current_percentage = 1.0 * self.event_number / self.total_number_of_events

        if 100 * current_percentage % 10 == 0:
            registered_store_arrays = Belle2.PyStoreArray.list()
            registered_store_objects = Belle2.PyStoreObj.list()

            event_store_content_list = []

            for store_array_name in registered_store_arrays:
                store_array = Belle2.PyStoreArray(store_array_name)
                event_store_content_list.append([store_array_name, len(store_array)])

            for store_array_name in registered_store_objects:
                event_store_content_list.append([store_array_name, 0])

            self.store_content_list.append({"number": self.event_number, "store_content": event_store_content_list})

        self.event_number += 1

    def terminate(self):
        """
        Write the store array contents from the events to the queue.
        """
        self.queue.put("basf2.store_content", self.store_content_list)


class ProgressPython(basf2.Module):

    """
    A small module that prints every now and then the event number to the given connection.
    It is used for status viewers. Do not call it by yourself.
    """

    def __init__(self, queue):
        """ Init the module """
        basf2.Module.__init__(self)
        self.queue = queue
        self.event_number = 0
        self.total_number_of_events = 0
        self.queue.send("init")

    def initialize(self):
        """ Send start to the connection """
        self.queue.send("start")

        # Receive the total number of events

        import ROOT
        self.total_number_of_events = ROOT.Belle2.Environment.Instance().getNumberOfEvents()

    def event(self):
        """ Send the event number to the connection """
        if self.total_number_of_events == 0:
            return

        current_percentage = 1.0 * self.event_number / self.total_number_of_events

        if 100 * current_percentage % 5 == 0:
            self.queue.send(current_percentage)

        self.event_number += 1

    def terminate(self):
        """ Send stop to the connection """
        self.queue.send(1)
        self.queue.send("end")
