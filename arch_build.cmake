###########################################################
#
# SENSO_APP platform build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the SENSO_APP configuration
set(SENSO_APP_PLATFORM_CONFIG_FILE_LIST
  senso_app_internal_cfg_values.h
  senso_app_platform_cfg.h
  senso_app_perfids.h
  senso_app_msgids.h
  senso_app_msgid_values.h
)

generate_configfile_set(${SENSO_APP_PLATFORM_CONFIG_FILE_LIST})

