###########################################################
#
# SENSO_APP mission build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the SENSO_APP configuration
set(SENSO_APP_MISSION_CONFIG_FILE_LIST
  senso_app_fcncode_values.h
  senso_app_interface_cfg_values.h
  senso_app_mission_cfg.h
  senso_app_perfids.h
  senso_app_msg.h
  senso_app_msgdefs.h
  senso_app_msgstruct.h
  senso_app_tbl.h
  senso_app_tbldefs.h
  senso_app_tblstruct.h
  senso_app_topicid_values.h
)

generate_configfile_set(${SENSO_APP_MISSION_CONFIG_FILE_LIST})

