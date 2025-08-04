#pragma once

#include "option.h"

#define FF_EDITOR_MODULE_NAME "Editor"

void ffPrintEditor(FFEditorOptions* options);
void ffInitEditorOptions(FFEditorOptions* options);
void ffDestroyEditorOptions(FFEditorOptions* options);

extern FFModuleBaseInfo ffEditorModuleInfo;
