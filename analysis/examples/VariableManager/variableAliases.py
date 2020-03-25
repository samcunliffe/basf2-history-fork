#!/usr/bin/env python3

# Tired of using long names like ``genUpsilon4S(daughter(0, daughter(0, PDG)))``
# (which become even less readable in its ROOT compatible form as demonstrated in
# variableManager.py)?
# Well, despair no longer! With so called "aliases", you can define handy short
# names for your complicated variables.
#
# Contributors: Kilian Lieret (2020)
#
# For full documentation please refer to https://software.belle2.org
# Anything unclear? Ask questions at https://questions.belle2.org
#

# vm is our shorthand name for the VariableManager instance
from variables import variables as vm

# More utilities for managing variables
import variables.utils as vu


# Let's start small and explicitly define alias for some simple variables:
# Let clusterECMS_{i} be the shorthand of daughter({i}, useCMSFrame(clusterE))
vm.addAlias('d0_x', 'daughter(0, x)')
vm.addAlias('d0_y', 'daughter(0, y)')
vm.addAlias('d0_z', 'daughter(0, z)')

# Now as you see, we have a clear naming convention in our head, but typing it
# all out will quickly become cumbersome (there's lot's of variables and lot's
# of daughters after all).
# Let's do the same thing with the momenta, but in just one line:
vu.create_aliases(
    list_of_variables=["px", "py", "pz"],
    wrapper="daughter(0, {variable})",
    prefix="d0_",
)
# {variable} is replaced by the name of each variable, so we just defined
# the aliases d0_px, d0_py, ...


# But in fact, this can be done even easier, because the variable utils provide
# as with a dedicated convenience function just to define aliases for
# daughter(.., ..) variables (it's very common after all)
# Let's create aliases for the second daughter

vu.create_daughter_aliases(
    list_of_variables=["x", "y", "z", "px", "py", "pz"],
    indices=1,
)

# In fact, we can also use create_daughter_aliases to create aliases for
# grand(grand(grand))-daughters:
vu.create_daughter_aliases(
    list_of_variables=["PDG"],
    indices=(0, 0, 0),
)
# This will create the alias
# d0_d0_d0_PDG -> daughter(0, daughter(0, daughter(0, PDG)))
# As before, you can also specify a prefix with the prefix keyword argument.

# Another similarly easy convenience function exists for MC truth variables,
# i.e. ``matchedMC(variable)``, which returns the value of the variable for the
# truth particle matched to the particle at hand.
vu.create_mctruth_aliases(
    list_of_variables=["x", "y", "z"],
)
# This creates mc_x, mc_y, etc. You can also customize the prefix with the
# optional prefix keyword argument.

# Let's finally visit one of the more complex convenience functions offered:
# create_aliases_for_selected(...). Let's say you're looking at the decay
# B0 -> [D0 -> pi+ K-] pi0 and want to create variables for the pi^+.
# Of course we could simply do that by hand or by using create_daughter_aliases
# (after all, the pi+ is simply daughter(0, daughter(0, ...))).
# But there's another way, directly from the decay string: Remember that you
# can mark a particle with ``^``: ``B0 -> [D0 -> pi+ K-] pi0``.
# We use that annotation to tell the variable utils which alias we want to
# create:
vu.create_aliases_for_selected(
    list_of_variables=["x", "y", "z"],
    decay_string="B0 -> [D0 -> ^pi+ K-] pi0",
)
# The shorthand name that will be defined by this is by default
# the names of all parent particle names separated by a underscore (if unique).
# If there are more particles with the same name, the daughter indices will
# be appended.
# We can also choose to use a similar naming convention as before:
# d0_d0_... (using daughter indices). For this, add use_names=False to the
# options.
# Take a look at the documentation for create_aliases_for_selected for more
# options!

# Finally let's print all of our alias
vm.printAliases()
