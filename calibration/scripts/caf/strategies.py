#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import B2ERROR, B2FATAL, B2INFO
from .utils import AlgResult
from .utils import B2INFO_MULTILINE
from .utils import runs_overlapping_iov, runs_from_vector
from .utils import iov_from_runs, split_runs_by_exp
from .utils import find_gaps_in_iov_list, grouper
from .utils import IoV, ExpRun
from .state_machines import AlgorithmMachine

from abc import ABC, abstractmethod
import itertools


class AlgorithmStrategy(ABC):
    """
    Base class for Algorithm strategies. These do the actual execution of a single
    algorithm on collected data. Each strategy may be quite different in terms of how fast it may be,
    how database payloads are passed between executions, and whether or not final payloads have an IoV
    that is independent to the actual runs used to calculates them.

    Parameters:
        algorithm (:py:class:`caf.framework.Algorithm`): The algorithm we will run

    This base class defines the basic attributes and methods that will be automatically used by the selected AlgorithmRunner.
    When defining a derived class you are free to use these attributes or to implement as much functionality as you want.

    If you define your derived class with an __init__ method, then you should first call the base class
    `AlgorithmStrategy.__init__()`  method via super() e.g.

    >>> def __init__(self):
    >>>     super().__init__()

    The most important method to implement is :py:meth:`AlgorithmStrategy.run` which will take an algorithm and execute it
    in the required way defined by the options you have selected/attributes set.
    """
    #: Required attributes that must exist before the strategy can run properly.
    #: Some are allowed be values that return False whe tested e.g. "" or []
    required_attrs = ["algorithm",
                      "database_chain",
                      "dependent_databases",
                      "output_dir",
                      "output_database_dir",
                      "input_files",
                      "ignored_runs"
                      ]

    #: Attributes that must have a value that returns True when tested by :py:meth:`is_valid`.
    required_true_attrs = ["algorithm",
                           "output_dir",
                           "output_database_dir",
                           "input_files"
                           ]

    #: Granularity of collector that can be run by this algorithm properly
    allowed_granularities = ["run", "all"]

    #: Signal value that is put into the Queue when there are no more results left
    FINISHED_RESULTS = "DONE"

    def __init__(self, algorithm):
        """
        """
        #: Algorithm() class that we're running
        self.algorithm = algorithm
        #: Collector output files, will contain all files retured by the output patterns
        self.input_files = []
        #: The algorithm output directory which is mostly used to store the stdout file
        self.output_dir = ""
        #: The output database directory for the localdb that the algorithm will commit to
        self.output_database_dir = ""
        #: User defined database chain i.e. the default global tag, or if you have localdb's/tags for custom alignment etc
        self.database_chain = []
        #: CAF created local databases from previous calibrations that this calibration/algorithm depends on
        self.dependent_databases = []
        #: Runs that will not be included in ANY execution of the algorithm. Usually set by Calibration.ignored_runs.
        #: The different strategies may handle the resulting run gaps differently.
        self.ignored_runs = []
        #: The list of results objects which will be sent out before the end
        self.results = []
        #: The multiprocessing Queue we use to pass back results one at a time
        self.queue = None

    @abstractmethod
    def run(self, iov, iteration, queue):
        """
        Abstract method that needs to be implemented. It will be called to actually execute the
        algorithm.
        """
        pass

    def setup_from_dict(self, params):
        """
        Parameters:
            params (dict): Dictionary containing values to be assigned to the strategy attributes of the same name.
        """
        for attribute_name, value in params.items():
            setattr(self, attribute_name, value)

    def is_valid(self):
        """
        Returns:
            bool: Whether or not this strategy has been set up correctly with all its necessary attributes.
        """
        B2INFO("Checking validity of current AlgorithmStrategy setup.")
        # Check if we're somehow missing a required attribute (should be impossible since they get initialised in init)
        for attribute_name in self.required_attrs:
            if not hasattr(self, attribute_name):
                B2ERROR("AlgorithmStrategy attribute {} doesn't exist.".format(attribute_name))
                return False
        # Check if any attributes that need actual values haven't been set or were empty
        for attribute_name in self.required_true_attrs:
            if not getattr(self, attribute_name):
                B2ERROR("AlgorithmStrategy attribute {} returned False.".format(attribute_name))
                return False
        return True

    def find_iov_gaps(self):
        """
        Finds and prints the current gaps between the IoVs of the strategy results. Basically these are the IoVs
        not covered by any payload. It CANNOT find gaps if they exist across an experiment boundary. Only gaps
        within the same experiment are found.

        Returns:
            iov_gaps(list[IoV])
        """
        iov_gaps = find_gaps_in_iov_list(sorted([result.iov for result in self.results]))
        if iov_gaps:
            gap_msg = ["Found gaps between IoVs of algorithm results (regardless of result)."]
            gap_msg.append("You may have requested these gaps deliberately by not passing in data containing these runs.")
            gap_msg.append("This may not be a problem, but you will not have payoads defined for these IoVs")
            gap_msg.append("unless you edit the final database.txt yourself.")
            B2INFO_MULTILINE(gap_msg)
            for iov in iov_gaps:
                B2INFO("{} not covered by any execution of the algorithm.".format(iov))
        return iov_gaps


class SingleIOV(AlgorithmStrategy):
    """The fastest and simplest Algorithm strategy. Runs the algorithm only once over all of the input
    data or only the data corresponding to the requested IoV. The payload IoV is the set to the same as the one
    that was executed.

    This uses a `caf.state_machines.AlgorithmMachine` to actually execute the various steps rather than operating on
    a CalibrationAlgorithm C++ class directly.
"""
    #: The params that you could set on the Algorithm object which this Strategy would use.
    #: Just here for documentation reasons.
    usable_params = {"apply_iov": IoV}

    def __init__(self, algorithm):
        """
        """
        super().__init__(algorithm)
        #: :py:class:`caf.state_machines.AlgorithmMachine` used to help set up and execute CalibrationAlgorithm
        #: It gets setup properly in :py:func:`run`
        self.machine = AlgorithmMachine(self.algorithm)

    def run(self, iov, iteration, queue):
        """
        Runs the algorithm machine over the collected data and fills the results.
        """
        if not self.is_valid():
            raise StrategyError("This AlgorithmStrategy was not set up correctly!")
        self.queue = queue

        B2INFO("Setting up {} strategy for {}".format(self.__class__.__name__, self.algorithm.name))
        # Now add all the necessary parameters for a strategy to run
        machine_params = {}
        machine_params["database_chain"] = self.database_chain
        machine_params["dependent_databases"] = self.dependent_databases
        machine_params["output_dir"] = self.output_dir
        machine_params["output_database_dir"] = self.output_database_dir
        machine_params["input_files"] = self.input_files
        machine_params["ignored_runs"] = self.ignored_runs
        self.machine.setup_from_dict(machine_params)
        # Start moving through machine states
        B2INFO("Starting AlgorithmMachine of {}".format(self.algorithm.name))
        self.machine.setup_algorithm(iteration=iteration)
        # After this point, the logging is in the stdout of the algorithm
        B2INFO("Beginning execution of {} using strategy {}".format(self.algorithm.name, self.__class__.__name__))

        all_runs_collected = set(runs_from_vector(self.algorithm.algorithm.getRunListFromAllData()))
        # If we were given a specific IoV to calibrate we just execute all runs in that IoV at once
        if iov:
            runs_to_execute = runs_overlapping_iov(iov, all_runs_collected)
        else:
            runs_to_execute = all_runs_collected

        # Remove the ignored runs from our run list to execute
        if self.ignored_runs:
            B2INFO("Removing the ignored_runs from the runs to execute for {}".format(self.algorithm.name))
            runs_to_execute.difference_update(set(self.ignored_runs))
        # Sets aren't ordered so lets go back to lists and sort
        runs_to_execute = sorted(runs_to_execute)
        apply_iov = None
        if "apply_iov" in self.algorithm.params:
            apply_iov = self.algorithm.params["apply_iov"]
        self.machine.execute_runs(runs=runs_to_execute, iteration=iteration, apply_iov=apply_iov)
        B2INFO("Finished execution with result code {}".format(self.machine.result.result))

        if (self.machine.result.result == AlgResult.ok.value) or (self.machine.result.result == AlgResult.iterate.value):
            # Valid exit codes mean we can complete properly
            self.machine.complete()
            # Commit all the payloads and send out the results
            self.machine.algorithm.algorithm.commit()
        else:
            # Either there wasn't enough data or the algorithm failed
            self.machine.fail()
        # No real reason for this, just so we have it in case of code changes later
        self.results.append(self.machine.result)
        # Send out the result
        self.queue.put(self.machine.result)
        self.queue.put(self.FINISHED_RESULTS)


class SequentialRunByRun(AlgorithmStrategy):
    """
    Algorithm strategy to do run-by-run calibration of collected data.
    Runs the algorithm over the input data contained within the requested IoV, starting with the first run's data only.
    If the algorithm returns 'not enough data' on the current run set, it won't commit the payloads, but instead adds
    the next run's data and tries again.

    Once an execution on a set of runs return 'iterate' or 'ok' we move onto the next runs (if any are left)
    and start the same procedure again. Committing of payloads to the outputdb only happens once we're sure that there
    is enough data in the remaining runs to get a full execution. If there isn't enough data remaining, the last runs
    are merged with the previous successful execution's runs and a final execution is performed on all remaining runs.

    This uses a `caf.state_machines.AlgorithmMachine` to actually execute the various steps rather than operating on
    a CalibrationAlgorithm C++ class directly.
"""
    #: The params that you could set on the Algorithm object which this Strategy would use.
    #: Just here for documentation reasons.
    usable_params = {
        "iov_coverage": IoV,
        "step_size": int
    }

    #: Granularity of collector that can be run by this algorithm properly
    allowed_granularities = ["run"]

    def __init__(self, algorithm):
        """
        """
        super().__init__(algorithm)
        #: :py:class:`caf.state_machines.AlgorithmMachine` used to help set up and execute CalibrationAlgorithm
        #: It gets setup properly in :py:func:`run`
        self.machine = AlgorithmMachine(self.algorithm)
        if "step_size" not in self.algorithm.params:
            self.algorithm.params["step_size"] = 1
        self.first_execution = True

    def run(self, iov, iteration, queue):
        """
        Runs the algorithm machine over the collected data and fills the results.
        """
        if not self.is_valid():
            raise StrategyError("This AlgorithmStrategy was not set up correctly!")
        self.queue = queue
        B2INFO("Setting up {} strategy for {}".format(self.__class__.__name__, self.algorithm.name))
        # Now add all the necessary parameters for a strategy to run
        machine_params = {}
        machine_params["database_chain"] = self.database_chain
        machine_params["dependent_databases"] = self.dependent_databases
        machine_params["output_dir"] = self.output_dir
        machine_params["output_database_dir"] = self.output_database_dir
        machine_params["input_files"] = self.input_files
        machine_params["ignored_runs"] = self.ignored_runs
        self.machine.setup_from_dict(machine_params)
        # Start moving through machine states
        self.machine.setup_algorithm(iteration=iteration)
        # After this point, the logging is in the stdout of the algorithm
        B2INFO("Beginning execution of {} using strategy {}".format(self.algorithm.name, self.__class__.__name__))
        runs_to_execute = []
        all_runs_collected = runs_from_vector(self.algorithm.algorithm.getRunListFromAllData())
        # If we were given a specific IoV to calibrate we just execute over runs in that IoV
        if iov:
            runs_to_execute = runs_overlapping_iov(iov, all_runs_collected)
        else:
            runs_to_execute = all_runs_collected[:]

        # Remove the ignored runs from our run list to execute
        if self.ignored_runs:
            B2INFO("Removing the ignored_runs from the runs to execute for {}".format(self.algorithm.name))
            runs_to_execute.difference_update(set(self.ignored_runs))
        # Sets aren't ordered so lets go back to lists and sort
        runs_to_execute = sorted(runs_to_execute)

        # We don't want to cross the boundary of Experiments accidentally. So we will split our run list
        # into separate lists, one for each experiment number contained. That way we can evaluate each experiment
        # separately and prevent IoVs from crossing the boundary.
        runs_to_execute = split_runs_by_exp(runs_to_execute)

        # Now iterate through the experiments, executing runs in blocks of 'step_size'. We DO NOT allow a payload IoV to
        # extend over multiple experiments, only multiple runs
        iov_coverage = None
        if "iov_coverage" in self.algorithm.params:
            B2INFO("Detected that you have set iov_coverage to {}".format(self.algorithm.params["iov_coverage"]))
            iov_coverage = self.algorithm.params["iov_coverage"]

        number_of_experiments = len(runs_to_execute)
        # Iterate over experiment run lists
        for i_exp, run_list in enumerate(runs_to_execute, start=1):
            # If 'iov_coverage' was set in the algorithm.params and it is larger (at both ends) than the
            # input data runs IoV, then we also have to set the first payload IoV to encompass the missing beginning
            # of the iov_coverage, and the last payload IoV must cover up to the end of iov_coverage.
            # This is only true for the lowest and highest experiments in our input data.
            if iov_coverage and i_exp == 1:
                lowest_exprun = ExpRun(iov_coverage.exp_low, iov_coverage.run_low)
            else:
                lowest_exprun = run_list[0]

            if iov_coverage and i_exp == number_of_experiments:
                highest_exprun = ExpRun(iov_coverage.exp_high, iov_coverage.run_high)
            else:
                highest_exprun = run_list[-1]

            self.execute_over_run_list(iteration, run_list, lowest_exprun, highest_exprun)
        # Print any knowable gaps between result IoVs, if any are foun there is a problem.
        self.find_iov_gaps()

    def execute_over_run_list(self, iteration, run_list, lowest_exprun, highest_exprun):
        # The runs (data) we have left to execute from this run list
        remaining_runs = run_list[:]
        # The previous execution's runs
        previous_runs = []
        # The current runs we are executing
        current_runs = []
        # The last successful payload and result
        last_successful_payloads = None
        last_successful_result = None

        # Iterate over ExpRuns within an experiment in chunks of 'step_size'
        for expruns in grouper(self.algorithm.params["step_size"], run_list):
            # Already set up earlier the first time, so we shouldn't do it again
            if not self.first_execution:
                self.machine.setup_algorithm()
            else:
                self.first_execution = False

            # Add on the next step of runs
            current_runs.extend(expruns)
            # Remove them from our remaining runs
            remaining_runs = [run for run in remaining_runs if run not in current_runs]

            # Is this the first payload of the experiment
            if not last_successful_result:
                B2INFO("Detected that this will be the first payload of this experiment.")
                # If this is the first payload but we have other data, we need the IoV to cover from the
                # lowest IoV extent requested up to the ExpRun right before the next run in the remaining runs list.
                if remaining_runs:
                    apply_iov = IoV(*lowest_exprun, remaining_runs[0].exp, remaining_runs[0].run - 1)
                # If this is the first payload but there isn't more data, we set the IoV to cover the full range
                else:
                    B2INFO("Detected that this will be the only payload of the experiment.")
                    apply_iov = IoV(*lowest_exprun, *highest_exprun)
            # If there were previous successes
            else:
                if not remaining_runs:
                    B2INFO("Detected that there are no more runs to execute in this experiment after this next execution.")
                    apply_iov = IoV(*current_runs[0], *highest_exprun)
                # Othewise, it's just a normal IoV in the middle.
                else:
                    B2INFO("Detected that there are more runs to execute in this experiment after this next execution.")
                    apply_iov = IoV(*current_runs[0], remaining_runs[0].exp, remaining_runs[0].run - 1)

            B2INFO("Executing and applying {} to the payloads".format(apply_iov))
            self.machine.execute_runs(runs=current_runs, iteration=iteration, apply_iov=apply_iov)
            B2INFO("Finished execution with result code {}".format(self.machine.result.result))

            # Does this count as a successful execution?
            if (self.machine.result.result == AlgResult.ok.value) or (self.machine.result.result == AlgResult.iterate.value):
                self.machine.complete()
                # If we've succeeded but we have a previous success we can commit the previous payloads
                # since we have new ones ready
                if last_successful_payloads and last_successful_result:
                    B2INFO("Saving this execution's payloads to be committed later.")
                    # Save the payloads and result
                    new_successful_payloads = self.machine.algorithm.algorithm.getPayloadValues()
                    new_successful_result = self.machine.result
                    B2INFO("We just succeded in execution of the Algorithm."
                           " Will now commit payloads from the previous success for {}.".format(last_successful_result.iov))
                    self.machine.algorithm.algorithm.commit(last_successful_payloads)
                    self.results.append(last_successful_result)
                    self.queue.put(last_successful_result)
                    # If there are remaining runs we need to have the current payloads ready to commit after the next execution
                    if remaining_runs:
                        last_successful_payloads = new_successful_payloads
                        last_successful_result = new_successful_result
                    # If there's not more runs to process we should also commit the new ones
                    else:
                        B2INFO("We have no more runs to process. "
                               "Will now commit the most recent payloads for {}".format(new_successful_result.iov))
                        self.machine.algorithm.algorithm.commit(new_successful_payloads)
                        self.results.append(new_successful_result)
                        self.queue.put(new_successful_result)
                        break
                # if there's no previous success this must be the first run executed
                else:
                    # Need to save payloads for later if we have a success but runs remain
                    if remaining_runs:
                        B2INFO("Saving the most recent payloads for {} to be committed later.".format(self.machine.result.iov))
                        # Save the payloads and result
                        last_successful_payloads = self.machine.algorithm.algorithm.getPayloadValues()
                        last_successful_result = self.machine.result
                    # Need to commit and exit if we have a success and no remaining data
                    else:
                        B2INFO("We just succeeded in execution of the Algorithm."
                               " No runs left to be processed, so we are committing results of this execution.")
                        self.machine.algorithm.algorithm.commit()
                        self.results.append(self.machine.result)
                        self.queue.put(self.machine.result)
                        break

                previous_runs = current_runs[:]
                current_runs = []
            # If it wasn't successful, was it due to lack of data in the runs?
            elif (self.machine.result.result == AlgResult.not_enough_data.value):
                B2INFO("There wasn't enough data in {}".format(self.machine.result.iov))
                if remaining_runs:
                    B2INFO("Some runs remain to be processed. "
                           "Will try to add at most {} more runs of data and execute again."
                           "".format(self.algorithm.params["step_size"]))
                elif not remaining_runs and not last_successful_result:
                    B2ERROR("There aren't any more runs remaining to merge with, and we never had a previous success."
                            " There wasn't enough data in the full input data requested.")
                    self.results.appemd(self.machine.result)
                    self.queue.put(self.machine.result)
                    self.machine.fail()
                    break
                elif not remaining_runs and last_successful_result:
                    B2INFO("There aren't any more runs remaining to merge with. But we had a previous success"
                           ", so we'll merge with the previous IoV.")
                    final_runs = current_runs[:]
                    current_runs = previous_runs
                    current_runs.extend(final_runs)
                self.machine.fail()
            elif self.machine.result.result == AlgResult.failure.value:
                B2ERROR("{} returned failure exit code.".format(self.algorithm.name))
                self.results.append(self.machine.result)
                self.queue.put(self.machine.result)
                self.machine.fail()
                break
        else:
            # Check if we need to run a final execution on the previous execution + dangling set of runs
            if current_runs:
                self.machine.setup_algorithm()
                apply_iov = IoV(last_successful_result.iov.exp_low,
                                last_successful_result.iov.run_low,
                                *highest_exprun)
                B2INFO("Executing on {}".format(apply_iov))
                self.machine.execute_runs(runs=current_runs, iteration=iteration, apply_iov=apply_iov)
                B2INFO("Finished execution with result code {}".format(self.machine.result.result))
                if (self.machine.result.result == AlgResult.ok.value) or (
                        self.machine.result.result == AlgResult.iterate.value):
                    self.machine.complete()
                    # Commit all the payloads and send out the results
                    self.machine.algorithm.algorithm.commit()
                    # Save the result
                    self.results.append(self.machine.result)
                    self.queue.put(self.machine.result)
                else:
                    # Save the result
                    self.results.append(self.machine.result)
                    self.queue.put(self.machine.result)
                    # But failed
                    self.machine.fail()
        self.queue.put(self.FINISHED_RESULTS)


class SimpleRunByRun(AlgorithmStrategy):
    """
    Algorithm strategy to do run-by-run calibration of collected data.
    Runs the algorithm over the input data contained within the requested IoV, starting with the first run's data only.

    This strategy differs from `SequentialRunByRun` in that it *will not merge run data* if the algorithm returns
    'not enough data' on the current run.

    Once an execution on a run returns *any* result 'iterate', 'ok', 'not_enough_data', or 'failure', we move onto the
    next run (if any are left).
    Committing of payloads to the outputdb only happens for 'iterate' or 'ok' return codes.

    .. important:: Unlike most other strategies, this one won't immediately fail and return if a run returns a 'failure' exit
                   code.
                   The failure will prevent iteration/successful completion of the CAF though.

    .. warning:: Since this strategy doesn't try to merge data from runs, if *any* run in your input data doesn't contain
                 enough data to complete the algorithm successfully, you won't be able to get a successful calibration.
                 The CAF then won't allow you to iterate this calibration, or pass the constants onward to another calibration.
                 However, you will still have the database created that covers all the successfull runs.

    This uses a `caf.state_machines.AlgorithmMachine` to actually execute the various steps rather than operating on
    a CalibrationAlgorithm C++ class directly.
"""

    allowed_granularities = ["run"]

    def __init__(self, algorithm):
        """
        """
        super().__init__(algorithm)
        #: :py:class:`caf.state_machines.AlgorithmMachine` used to help set up and execute CalibrationAlgorithm
        #: It gets setup properly in :py:func:`run`
        self.machine = AlgorithmMachine(self.algorithm)

    def run(self, iov, iteration, queue):
        """
        Runs the algorithm machine over the collected data and fills the results.
        """
        if not self.is_valid():
            raise StrategyError("This AlgorithmStrategy was not set up correctly!")
        self.queue = queue
        B2INFO("Setting up {} strategy for {}".format(self.__class__.__name__, self.algorithm.name))
        # Now add all the necessary parameters for a strategy to run
        machine_params = {}
        machine_params["database_chain"] = self.database_chain
        machine_params["dependent_databases"] = self.dependent_databases
        machine_params["output_dir"] = self.output_dir
        machine_params["output_database_dir"] = self.output_database_dir
        machine_params["input_files"] = self.input_files
        self.machine.setup_from_dict(machine_params)
        # Start moving through machine states
        self.machine.setup_algorithm(iteration=iteration)
        # After this point, the logging is in the stdout of the algorithm
        B2INFO("Beginning execution of {} using strategy {}".format(self.algorithm.name, self.__class__.__name__))
        runs_to_execute = []
        all_runs_collected = runs_from_vector(self.algorithm.algorithm.getRunListFromAllData())
        # If we were given a specific IoV to calibrate we just execute over runs in that IoV
        if iov:
            runs_to_execute = runs_overlapping_iov(iov, all_runs_collected)
        else:
            runs_to_execute = all_runs_collected[:]
        # The runs we have left to execute
        remaining_runs = runs_to_execute[:]
        # Is this the first time executing the algorithm?
        first_execution = True
        for exprun in runs_to_execute:
            if not first_execution:
                self.machine.setup_algorithm()
            current_runs = exprun
            # Remove current run from our remaining runs
            remaining_runs.pop(0)
            B2INFO("Executing on IoV = {}".format(iov_from_runs([current_runs])))
            self.machine.execute_runs(runs=[current_runs], iteration=iteration)
            first_execution = False
            B2INFO("Finished execution with result code {}".format(self.machine.result.result))
            # Does this count as a successful execution?
            if (self.machine.result.result == AlgResult.ok.value) or (self.machine.result.result == AlgResult.iterate.value):
                # Commit the payloads and result
                B2INFO("Committing payloads for {}.".format(iov_from_runs([current_runs])))
                self.machine.algorithm.algorithm.commit()
                self.results.append(self.machine.result)
                self.queue.put(self.machine.result)
                self.machine.complete()
            # If it wasn't successful, was it due to lack of data in the runs?
            elif (self.machine.result.result == AlgResult.not_enough_data.value):
                B2INFO("There wasn't enough data in the IoV {}".format(iov_from_runs([current_runs])))
                self.results.append(self.machine.result)
                self.queue.put(self.machine.result)
                self.machine.fail()
            elif self.machine.result.result == AlgResult.failure.value:
                B2ERROR("Failure exit code in the IoV {}".format(iov_from_runs([current_runs])))
                self.results.append(self.machine.result)
                self.queue.put(self.machine.result)
                self.machine.fail()
        self.queue.put(self.FINISHED_RESULTS)

        # Print any knowable gaps between result IoVs
        self.find_iov_gaps()


class StrategyError(Exception):
    """
    Basic Exception for this type of class
    """
    pass
