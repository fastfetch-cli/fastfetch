#pragma once

#include "option.h"

bool ffPrintEditor(FFEditorOptions* options);
void ffInitEditorOptions(FFEditorOptions* options);
void ffDestroyEditorOptions(FFEditorOptions* options);

extern FFModuleBaseInfo ffEditorModuleInfo;
