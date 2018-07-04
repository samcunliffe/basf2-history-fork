import basf2
import ROOT
from ROOT import Belle2
import numpy as np


class PerEventStatisticsGetterModule(basf2.Module):
    """
    A basf2 python module to export *all* module time statistics (of every event, not just averaged)
     into a ROOT TTree written to a file.
    """

    def __init__(self, output_file_name):
        """
        Create a new PerEventStatisticsGetterModule. You have to give the name of the
        file, where the TTree with the full event statistics will be saved.
        """
        #: Name of the output file.
        self.output_file_name = output_file_name

        #: Will host the Store Object Pointer to the ROOTMergeable later.
        self.statistics = None

        #: Will host the pointer to the opened TFile later.
        self.file = None

        #: The columns for the statistics TTree (they will be filled in the event function).
        self.ttree_inputs = None
        #: The column to store the proc ID (when doing multiprocessing).
        self.proc_id_input = np.zeros(1, dtype=float)
        #: The column to store the event number.
        self.event_number_input = np.zeros(1, dtype=float)

        #: A flag to indicate that we have already added the Branches to the TTree (which we will do in the first event)
        self.branches_added = False

        #: The event meta data
        self.event_meta_data = Belle2.PyStoreObj("EventMetaData")

        basf2.Module.__init__(self)

    def initialize(self):
        """
        Create the needed store object pointer in the DataStore and the TFile with the TTree.
        """
        # Create the object in the DataStore
        self.statistics = Belle2.PyStoreObj(Belle2.RootMergeable(ROOT.TTree).Class(),
                                            Belle2.DataStore.c_Persistent)
        self.statistics.registerInDataStore("PerEventStatistics")

        # Before creating the TTree, open the output file.
        self.file = ROOT.TFile.Open(self.output_file_name, "RECREATE")
        self.statistics.create()

        # Create the TTree.
        statistics_obj = self.statistics.obj()
        statistics_obj.assign(ROOT.TTree("statistics", "statistics"))

        # Add two branches we already know about.
        ttree = statistics_obj.get()
        ttree.Branch("proc_id", self.proc_id_input, "proc_id/D")
        ttree.Branch("event_number", self.event_number_input, "event_number/D")

        self.event_meta_data.isRequired()

    def event(self):
        """
        The event loop: Store the statistics as a new row in the TTree.
        """
        # Stored TTree in the DataStore.
        ttree = self.statistics.obj().get()

        # Get the statistics and the process ID.
        module_stats = basf2.statistics.modules
        proc_id = Belle2.ProcHandler.EvtProcID()

        if not self.branches_added:

            self.ttree_inputs = [np.zeros(1, dtype=float) for module in module_stats]

            for i, stat in enumerate(module_stats):
                # escape the module names in ROOT-safe manner. Otherwise weird stuff happens like
                # sub-branches get created or the branch cannot be opened in the TBrowser
                module_name = Belle2.makeROOTCompatible(stat.name)

                ttree.Branch(module_name + "_" + str(i), self.ttree_inputs[i], module_name + "_" + str(i) + "/D")

            self.branches_added = True

        # Fill in the module statistics into the branches
        for i, module_stat in enumerate(module_stats):
            self.ttree_inputs[i][0] = module_stat.time_sum(basf2.statistics.EVENT) - self.ttree_inputs[i][0]

        # Fill in the other two variables into the branches
        self.proc_id_input[0] = proc_id
        self.event_number_input[0] = self.event_meta_data.getEvent()

        # Send the branches to the TTree.
        ttree.Fill()

    def terminate(self):
        """
        Write out the merged statistics to the ROOT file.
        This should only be called once, as we would end up with different versions otherwise.
        """
        self.statistics.obj().write(self.file)
        self.file.Close()
