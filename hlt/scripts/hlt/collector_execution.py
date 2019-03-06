import os
import subprocess
import sys

from hlt.clean_execution import CleanBasf2Execution
from argparse import ArgumentParser


def main(script_name):
    """Shortcut method to do the basf2 execution on hltout"""
    os.chdir(sys.path[0])

    parser = ArgumentParser()
    parser.add_argument("input_ring_buffer", help="Name of the input ring buffer")
    parser.add_argument("output_ring_buffer", help="Name of the output ring buffer")
    parser.add_argument("--number-of-collectors", help="Number of collectors to start", default=3)

    args = parser.parse_args()

    input_ring_buffer_list = [f"{args.input_ring_buffer}{i + 1}" for i in range(0, args.number_of_collectors)]
    output_ring_buffer_list = [f"{args.output_ring_buffer}{i + 1}" for i in range(0, args.number_of_collectors)]
    message_queue_list = [f"/roi{i}" for i in range(0, args.number_of_collectors)]

    execution = CleanBasf2Execution()
    try:
        execution.start(["rb2mrb", args.input_ring_buffer] + input_ring_buffer_list)
        execution.start(["mrb2rb"] + output_ring_buffer_list + [args.output_ring_buffer])
        for input_rb, output_rb, roi_message_queue in zip(input_ring_buffer_list, output_ring_buffer_list,
                                                          message_queue_list):
            execution.start(["basf2", "--no-stats", script_name, "--", input_rb, output_rb, roi_message_queue])
        return_code = execution.wait()
    finally:
        execution.kill()
        subprocess.run(["framework-pcore-clear_ipcs"])
        # TODO: actually needed?
        for input_rb in input_ring_buffer_list:
            subprocess.run(["removerb", input_rb])
        for output_rb in output_ring_buffer_list:
            subprocess.run(["removerb", output_rb])

    exit(return_code)
