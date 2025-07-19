//#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "utils.h"
#include "ytdlp.h"
#include "sigact.h"
#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>

json_t *dl_ytdlp(struct ytdlp_opts_t *opts)
{
    
    PyGILState_STATE gstate = PyGILState_Ensure();

    json_error_t error;
    json_t *ret = NULL;
    PyObject *pDict = NULL;
    PyObject *pOuttmplDict = NULL;
    PyObject *pPostProcessList = NULL;
    PyObject *pKey1Dict = NULL;
    PyObject *pKey2Dict = NULL;
    PyObject *pKey3Dict = NULL;
    PyObject *pInstance = NULL;
    PyObject *pMethod = NULL;
    PyObject *pArgs = NULL;
    PyObject *pResult = NULL;
    PyObject *pSanitize = NULL;
    PyObject *pArgsSanitize = NULL;
    PyObject *pResultSanitized = NULL;
    PyObject *pJsonDumps = NULL;
    PyObject *pJsonArgs = NULL;
    PyObject *pKwargs = NULL;
    PyObject *pRepr = NULL;
    PyObject *pJsonString = NULL;

    // Add current directory to sys.path to ensure yt_dlp is found
    PyRun_SimpleString("import sys\nsys.path.append('"YT_DLP_PATH"')\n");
    PyRun_SimpleString("import sys\nsys.path.append('"VENV_SITE_PACKAGES"')\n");

    // Import yt_dlp module
    ctx->Py.pModule = PyImport_ImportModule("yt_dlp");
    if (!ctx->Py.pModule) {
        PyErr_Print();
        fprintf(stderr, "Failed to import yt_dlp\n");
        goto CleanUp;
    }
    ctx->Py.pJsonModule = PyImport_ImportModule("json");
    if (!ctx->Py.pJsonModule) {
        PyErr_Print();
        fprintf(stderr, "Failed to import json\n");
        goto CleanUp;
    }
    ctx->Py.pClass = PyObject_GetAttrString(ctx->Py.pModule, "YoutubeDL");
    if (!ctx->Py.pClass) {
        PyErr_Print();
        fprintf(stderr, "Failed to get YoutubeDL class\n");
        goto CleanUp;
    }

    pDict = PyDict_New();
    if (!pDict) {
        PyErr_Print();
        fprintf(stderr, "Failed to create options dictionary\n");
        goto CleanUp;
    }
    pOuttmplDict = PyDict_New();
    if (!pOuttmplDict) {
        PyErr_Print();
        fprintf(stderr, "Failed to create Outtmpl dictionary\n");
        goto CleanUp;
    }
    pPostProcessList = PyList_New(3);
    if (!pPostProcessList) {
        PyErr_Print();
        fprintf(stderr, "Failed to create PostProcess list\n");
        goto CleanUp;
    }

    pKey1Dict = PyDict_New();
    if (!pKey1Dict) {
        PyErr_Print();
        fprintf(stderr, "Failed to create Key1 dictionary\n");
        goto CleanUp;
    }
    PyDict_SetItemString(pKey1Dict, "key", PyUnicode_FromString("FFmpegVideoConvertor"));
    PyDict_SetItemString(pKey1Dict, "preferedformat", PyUnicode_FromString("mp4"));

    pKey2Dict = PyDict_New();
    if (!pKey2Dict) {
        PyErr_Print();
        fprintf(stderr, "Failed to create Key2 dictionary\n");
        goto CleanUp;
    }
    PyDict_SetItemString(pKey2Dict, "key", PyUnicode_FromString("FFmpegConcat"));
    PyDict_SetItemString(pKey2Dict, "only_multi_video", Py_True);
    PyDict_SetItemString(pKey2Dict, "when", PyUnicode_FromString("playlist"));

    pKey3Dict = PyDict_New();
    if (!pKey3Dict) {
        PyErr_Print();
        fprintf(stderr, "Failed to create Key3 dictionary\n");
        goto CleanUp;
    }
    PyDict_SetItemString(pKey3Dict, "already_have_subtitle", Py_False);
    PyDict_SetItemString(pKey3Dict, "key", PyUnicode_FromString("FFmpegEmbedSubtitle"));

    PyList_SetItem(pPostProcessList, 0, pKey1Dict);
    PyList_SetItem(pPostProcessList, 1, pKey2Dict);
    PyList_SetItem(pPostProcessList, 2, pKey3Dict);
    PyDict_SetItemString(pDict, "postprocessors", pPostProcessList);

    PyDict_SetItemString(pOuttmplDict, "default", PyUnicode_FromString(DL_PATH"/%(title)s [%(id)s].%(ext)s"));
    PyDict_SetItemString(pOuttmplDict, "thumbnail", PyUnicode_FromString(DL_PATH"/thumbnails/%(title)s [%(id)s].%(ext)s"));
    PyDict_SetItemString(pDict, "outtmpl", pOuttmplDict);
    PyDict_SetItemString(pDict, "extract_flat", PyUnicode_FromString("discard_in_playlist"));
    PyDict_SetItemString(pDict, "final_ext", PyUnicode_FromString("mp4"));
    PyDict_SetItemString(pDict, "merge_output_format", PyUnicode_FromString("mp4"));
    //PyDict_SetItemString(pDict, "format", PyUnicode_FromString("bestvideo[height<=1080]+bestaudio/best[height<=1080]"));
    PyDict_SetItemString(pDict, "format", PyUnicode_FromString("bestvideo[vcodec~='^hevc$|^avc1|^av1$'][ext=mp4]+bestaudio[ext=m4a]"));
    PyDict_SetItemString(pDict, "writethumbnail", Py_True);
    PyDict_SetItemString(pDict, "writesubtitles", Py_True);
    PyDict_SetItemString(pDict, "fragment_retries", PyLong_FromLong(10));
    PyDict_SetItemString(pDict, "retries", PyLong_FromLong(10));
    PyDict_SetItemString(pDict, "ignoreerrors", PyUnicode_FromString("only_download"));
    PyDict_SetItemString(pDict, "ffmpeg_location", PyUnicode_FromString(INSTALL_PREFIX"/bin/ffmpeg"));



    pInstance = PyObject_CallObject(ctx->Py.pClass, PyTuple_Pack(1, pDict));
    if (!pInstance) {
        PyErr_Print();
        fprintf(stderr, "Failed to instantiate YoutubeDL\n");
        goto CleanUp;
    }
    pMethod = PyObject_GetAttrString(pInstance, "extract_info");
    if (!pMethod) {
        PyErr_Print();
        fprintf(stderr, "Failed to get extract_info method\n");
        goto CleanUp;
    }

    pArgs = PyTuple_New(6);
    PyTuple_SetItem(pArgs, 0, PyUnicode_FromString(opts->url)); // url
    PyTuple_SetItem(pArgs, 1, Py_True); // download=True
    PyTuple_SetItem(pArgs, 2, Py_None);  // ie_key=None
    PyTuple_SetItem(pArgs, 3, Py_None);  // extra_info=None
    PyTuple_SetItem(pArgs, 4, Py_True);  // process=True
    PyTuple_SetItem(pArgs, 5, Py_False); // force_generic_extractor=False

    pResult = PyObject_CallObject(pMethod, pArgs);
    if (!pResult) {
        PyErr_Print();
        fprintf(stderr, "Failed to call extract_info\n");
        goto CleanUp;
    }

    pSanitize = PyObject_GetAttrString(pInstance, "sanitize_info");
    if (!pSanitize) {
        PyErr_Print();
        fprintf(stderr, "Failed to get sanitize_info method\n");
        goto CleanUp;
    }
    pArgsSanitize = PyTuple_New(1);
    PyTuple_SetItem(pArgsSanitize, 0, pResult);
    pResultSanitized = PyObject_CallObject(pSanitize, pArgsSanitize);
    if (!pResultSanitized) {
        PyErr_Print();
        fprintf(stderr, "Failed to call sanitize_info\n");
        goto CleanUp;
    }


    pJsonDumps = PyObject_GetAttrString(ctx->Py.pJsonModule, "dumps");
    if (!pJsonDumps) {
        PyErr_Print();
        fprintf(stderr, "Failed to get json.dumps\n");
        goto CleanUp;
    }

    // Call json.dumps(pResult, indent=2, ensure_ascii=False)
    pJsonArgs = PyTuple_New(1);
    PyTuple_SetItem(pJsonArgs, 0, pResultSanitized); // Borrowed reference

    pKwargs = PyDict_New();
    PyDict_SetItemString(pKwargs, "ensure_ascii", Py_False);

    pJsonString = PyObject_Call(pJsonDumps, pJsonArgs, pKwargs);
    if (!pJsonString || !PyUnicode_Check(pJsonString)) {
        PyErr_Print();
        fprintf(stderr, "Failed to serialize pResultSanitized to JSON\n");
        // Fallback to repr()
        pRepr = PyObject_Repr(pResultSanitized);
        if (pRepr && PyUnicode_Check(pRepr)) {
            printf("pResult (repr):\n%.*s\n", 100, PyUnicode_AsUTF8(pRepr));
        }
        if (pJsonString) {
            Py_XDECREF(pJsonString);
        }
        pJsonString = NULL;
    } else {
        printf("pResult (JSON):\n%.*s\n", 100, PyUnicode_AsUTF8(pJsonString));
    }
    if (pJsonString) {
        ret = json_loads(PyUnicode_AsUTF8(pJsonString), 0, &error);
    } else if (pRepr) {
        ret = json_loads(PyUnicode_AsUTF8(pRepr), 0, &error);
    } else {
        fprintf(stderr, "!pJsonString && !pRepr\n");
        goto CleanUp;
    }
    if (!ret) {
        fprintf(stderr, "json_loads failed: %s\n", error.text);
    }
CleanUp:
    if (pJsonString) Py_XDECREF(pJsonString);
    if (pKwargs) Py_DECREF(pKwargs);
    if (pJsonArgs) Py_DECREF(pJsonArgs);
    if (pJsonDumps) Py_DECREF(pJsonDumps);
    if (pResult) Py_DECREF(pResult);
    if (pResultSanitized) Py_DECREF(pResultSanitized);
    if (pArgs) Py_DECREF(pArgs);
    if (pArgsSanitize) Py_DECREF(pArgsSanitize);
    if (pMethod) Py_DECREF(pMethod);
    if (pSanitize) Py_DECREF(pSanitize);
    if (pInstance) Py_DECREF(pInstance);
    if (pDict) Py_DECREF(pDict);
    if (pOuttmplDict) Py_DECREF(pOuttmplDict);
    if (pPostProcessList) Py_DECREF(pPostProcessList);
    if (pKey1Dict) Py_DECREF(pKey1Dict);
    if (pKey2Dict) Py_DECREF(pKey2Dict);
    if (pKey3Dict) Py_DECREF(pKey3Dict);
    PyGILState_Release(gstate);  // Release GIL
    return ret;
}