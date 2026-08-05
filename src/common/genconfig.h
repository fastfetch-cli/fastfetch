#pragma once

#include "common/ffdata.h"

// Runs the interactive config generation CUI.
//
// On success (the user saved the config), it sets `data->structure` to the user chosen
// module structure and `instance.config.logo.type` to the user chosen logo type, creates
// the result document and returns true. The caller is expected to call `writeConfigFile`.
// Returns false if the user quits without saving.
bool ffGenConfigInteractive(FFdata* data);
