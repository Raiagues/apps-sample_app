project(CFE_SENSO_APP C)

set(APP_SRC_FILES
  fsw/src/senso_app.c
  fsw/src/senso_app_cmds.c
  fsw/src/senso_app_utils.c
)

if (CFE_EDS_ENABLED)
  list(APPEND APP_SRC_FILES
    fsw/src/senso_app_eds_dispatch.c
  )
else()
  list(APPEND APP_SRC_FILES
    fsw/src/senso_app_dispatch.c
  )
endif()

# Create the app module
add_cfe_app(senso_app ${APP_SRC_FILES})

target_include_directories(senso_app PUBLIC fsw/inc)
# Include the public API from sample_lib to demonstrate how
# to call library-provided functions
add_cfe_app_dependency(senso_app sample_lib)

# Add table
add_cfe_tables(senso_app fsw/tables/senso_app_tbl.c)

# If UT is enabled, then add the tests from the subdirectory
# Note that this is an app, and therefore does not provide
# stub functions, as other entities would not typically make
# direct function calls into this application.
if (ENABLE_UNIT_TESTS)
  add_subdirectory(unit-test)
endif (ENABLE_UNIT_TESTS)
