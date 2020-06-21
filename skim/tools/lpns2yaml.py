#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Convert list of LPNs into YAML format expected by b2skim-prod. The expected input to
this script is a text file of LPNs, which can be obtained from the dataset searcher.
"""

import argparse
import pandas as pd
from pathlib import Path
import re
import sys

from termcolor import colored
import yaml


__author__ = "Phil Grace"


def get_argument_parser():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        metavar="input_lpn_list_file",
        dest="input",
        help="Input file containing list of LPNs (such as that from the dataset searcher).",
    )
    parser.add_argument(
        "-o",
        metavar="output_filename",
        dest="output",
        help="Output YAML file name. If none given, prints to screen.",
    )
    DataMCGroup = parser.add_mutually_exclusive_group(required=True)
    DataMCGroup.add_argument("--data", action="store_true")
    DataMCGroup.add_argument("--mc", action="store_true")

    parser.add_argument("--bg", choices=("BGx0", "BGx1"), required=("--mc" in sys.argv))
    return parser


def verify_dataframe(df, mc):
    # Check that we got the names of the columns right
    if not (
        all(df["release"].str.startswith("release-"))
        and all(df["DBGT"].str.startswith("DB"))
        and all(df["expNumber"].str.startswith("e"))
        and all(df["runNumber"].str.startswith("r"))
    ):
        raise ValueError(
            "The column values don't seem to line up with what's expected."
        )

    # Check assumptions about columns
    if not all(df["dataLevel"].str.match("mdst")):
        raise ValueError("Input LPNs must all be mdst.")
    if mc and len(set(df["runNumber"])) > 1:
        raise ValueError("More than one run number listed for MC LPNs.")
    if len(set(df["beamEnergy"])) > 1:
        # Only print a warning
        print(
            colored(
                "[WARNING] Input LPNs contain more than one beam energy.", "yellow"
            ),
            file=sys.stderr,
        )


def to_yaml(data, output=None):
    """Print to screen or file as YAML format."""
    if isinstance(output, (str, Path)):
        OutputFilename = Path(output).with_suffix(".yaml")
        with open(OutputFilename, "w") as f:
            yaml.dump(data, f, sort_keys=False)
        print(f"[INFO] Wrote YAML file to {OutputFilename}", file=sys.stderr)
    elif not output:
        yaml.dump(data, sys.stdout, sort_keys=False)


def main():
    parser = get_argument_parser()
    args = parser.parse_args()

    # Read in data
    with open(args.input) as f:
        LPNs = sorted(filter(None, f.read().split("\n")))

    # Trim prefix (everything up to the release number)
    prefixes = {re.match(r"^(.*)(/release)", LPN)[1] for LPN in LPNs}
    if len(prefixes) > 1:
        raise ValueError("Somehow got multiple different prefixes!")
    else:
        prefix = list(prefixes)[0]
    LPNs = [re.sub(r"^.*(release)", r"\1", LPN) for LPN in LPNs]

    # ...and put it all into a lovely dataframe, split by LPN part!
    df = pd.DataFrame([Path(LPN).parts for LPN in LPNs])
    if args.data:
        columns = dict(
            enumerate(
                [
                    "release",
                    "DBGT",
                    "campaign",
                    "prodNumber",
                    "expNumber",
                    "beamEnergy",
                    "runNumber",
                    "dataLevel",
                ]
            )
        )
    else:
        columns = dict(
            enumerate(
                [
                    "release",
                    "DBGT",
                    "campaign",
                    "prodNumber",
                    "s00",
                    "expNumber",
                    "beamEnergy",
                    "runNumber",
                    "MCEventType",
                    "dataLevel",
                ]
            )
        )

    df.rename(columns=columns, inplace=True)
    verify_dataframe(df, args.mc)

    if args.mc:
        df.loc[:, "expNumber"] = df["s00"] + "/" + df["expNumber"]
        df.drop("s00", axis=1, inplace=True)

    DataBlocks = {}

    # Group into blocks, to make sure everything in block has the same prodID, release, etc.
    if args.data:
        ExpGroups = df.groupby(["campaign", "expNumber"])
        for (campaign, expNumber), ExpGroup in ExpGroups:
            groups = ExpGroup.groupby(["beamEnergy", "release", "DBGT", "prodNumber"])
            for (iGroup, ((beamEnergy, release, DBGT, prodNumber), group)) in enumerate(
                groups
            ):
                # Extract integers from columns
                prodNumber = int(re.sub(r"^prod0*", "", prodNumber))
                DBGT = int(re.sub(r"^DB0*", "", DBGT))
                expInteger = int(re.sub(r"^e0*", "", expNumber))

                # If beam energy is not 4S, then point it out in label
                onres = beamEnergy == "4S"
                if onres:
                    label = f"{campaign}_exp{expInteger}r{iGroup+1}"
                else:
                    label = f"{campaign}_{beamEnergy}_exp{expInteger}r{iGroup+1}"

                # Add everything to our mega dict
                DataBlocks[label] = {
                    "sampleLabel": (f"{campaign}_exp{expInteger}" if onres else "???"),
                    "LPNPrefix": prefix,
                    "inputReleaseNumber": release,
                    "prodNumber": prodNumber,
                    "inputDBGlobalTag": DBGT,
                    "campaign": campaign,
                    "experimentNumber": expNumber,
                    "beamEnergy": beamEnergy,
                    "inputDataLevel": "mdst",
                    "runNumbers": list(group["runNumber"]),
                }
    else:
        # Extract integers from columns
        df.loc[:, "prodNumber"] = (
            df["prodNumber"].str.replace("^prod0*", "").astype(int)
        )

        MCTypeGroups = df.groupby(["campaign", "MCEventType"])
        for (campaign, MCEventType), MCTypeGroup in MCTypeGroups:
            groups = MCTypeGroup.groupby(
                ["beamEnergy", "expNumber", "release", "DBGT", "runNumber"]
            )
            for (
                iGroup,
                ((beamEnergy, expNumber, release, DBGT, runNumber), group),
            ) in enumerate(groups):
                # Extract integers from columns
                DBGT = int(re.sub(r"^DB0*", "", DBGT))

                # If beam energy is not 4S, then point it out in label
                label = f"{campaign}_{MCEventType}{args.bg}"
                BlockLabel = f"{label}r{iGroup+1}"

                # Add everything to our mega dict
                DataBlocks[BlockLabel] = {
                    "sampleLabel": label,
                    "LPNPrefix": prefix,
                    "inputReleaseNumber": release,
                    "mcCampaign": campaign,
                    "prodNumber": list(group["prodNumber"]),
                    "inputDBGlobalTag": DBGT,
                    "experimentNumber": expNumber,
                    "beamEnergy": beamEnergy,
                    "mcType": MCEventType,
                    "mcBackground": args.bg,
                    "inputDataLevel": "mdst",
                    "runNumbers": runNumber,
                }

    to_yaml(DataBlocks, args.output)


if __name__ == "__main__":
    main()
    print(
        colored(
            (
                "[WARNING] Please check that the 'sampleLabel' entries in the output "
                "YAML file match sample labels in skim/scripts/TestFiles.yaml."
            ),
            "yellow",
        ),
        file=sys.stderr,
    )
