
#include "utils.h"
#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>

json_t *dl_ytdlp(char *url)
{
    json_error_t error;
    json_t *ret = NULL;

    Py_Initialize();

    // Add current directory to sys.path to ensure yt_dlp is found
    PyRun_SimpleString("import sys\nsys.path.append('"YT_DLP_PATH"')\n");
    PyRun_SimpleString("import sys\nsys.path.append('"VENV_SITE_PACKAGES"')\n");

    // Import yt_dlp module
    PyObject *pModule = PyImport_ImportModule("yt_dlp");
    if (!pModule) {
        PyErr_Print();
        fprintf(stderr, "Failed to import yt_dlp\n");
        Py_Finalize();
        return NULL;
    }
    PyObject *pClass = PyObject_GetAttrString(pModule, "YoutubeDL");
    if (!pClass) {
        PyErr_Print();
        fprintf(stderr, "Failed to get YoutubeDL class\n");
        Py_DECREF(pModule);
        Py_Finalize();
        return NULL;
    }
    PyObject *pDict = PyDict_New();
    if (!pDict) {
        PyErr_Print();
        fprintf(stderr, "Failed to create options dictionary\n");
        Py_DECREF(pClass);
        Py_DECREF(pModule);
        Py_Finalize();
        return NULL;
    }
    PyDict_SetItemString(pDict, "extract_flat", PyUnicode_FromString("discard_in_playlist"));
    PyDict_SetItemString(pDict, "format", PyUnicode_FromString("bestvideo[height<=1080]+bestaudio/best[height<=1080]"));
    PyDict_SetItemString(pDict, "writethumbnail", Py_True);
    PyDict_SetItemString(pDict, "fragment_retries", PyLong_FromLong(10));
    PyDict_SetItemString(pDict, "retries", PyLong_FromLong(10));
    PyDict_SetItemString(pDict, "ignoreerrors", PyUnicode_FromString("only_download"));
    PyDict_SetItemString(pDict, "ffmpeg_location", PyUnicode_FromString(INSTALL_PREFIX"/bin/ffmpeg"));

    PyObject *pInstance = PyObject_CallObject(pClass, PyTuple_Pack(1, pDict));
    if (!pInstance) {
        PyErr_Print();
        fprintf(stderr, "Failed to instantiate YoutubeDL\n");
        Py_DECREF(pDict);
        Py_DECREF(pClass);
        Py_DECREF(pModule);
        Py_Finalize();
        return NULL;
    }
    PyObject *pMethod = PyObject_GetAttrString(pInstance, "extract_info");
    if (!pMethod) {
        PyErr_Print();
        fprintf(stderr, "Failed to get extract_info method\n");
        Py_DECREF(pInstance);
        Py_DECREF(pDict);
        Py_DECREF(pClass);
        Py_DECREF(pModule);
        Py_Finalize();
        return NULL;
    }

    PyObject *pArgs = PyTuple_New(6);
    PyTuple_SetItem(pArgs, 0, PyUnicode_FromString(url)); // url
    PyTuple_SetItem(pArgs, 1, Py_True); // download=True
    PyTuple_SetItem(pArgs, 2, Py_None);  // ie_key=None
    PyTuple_SetItem(pArgs, 3, Py_None);  // extra_info=None
    PyTuple_SetItem(pArgs, 4, Py_True);  // process=True
    PyTuple_SetItem(pArgs, 5, Py_False); // force_generic_extractor=False

    PyObject *pResult = PyObject_CallObject(pMethod, pArgs);
    if (!pResult) {
        PyErr_Print();
        fprintf(stderr, "Failed to call extract_info\n");
        Py_DECREF(pArgs);
        Py_DECREF(pMethod);
        Py_DECREF(pInstance);
        Py_DECREF(pDict);
        Py_DECREF(pClass);
        Py_DECREF(pModule);
        Py_Finalize();
        return NULL;
    }

    PyObject *pSanitize = PyObject_GetAttrString(pInstance, "sanitize_info");
    if (!pSanitize) {
        PyErr_Print();
        fprintf(stderr, "Failed to get sanitize_info method\n");
        Py_DECREF(pMethod);
        Py_DECREF(pInstance);
        Py_DECREF(pDict);
        Py_DECREF(pClass);
        Py_DECREF(pModule);
        Py_Finalize();
        return NULL;
    }
    PyObject *pArgsSanitize = PyTuple_New(1);
    PyTuple_SetItem(pArgsSanitize, 0, pResult);
    PyObject *pResultSanitized = PyObject_CallObject(pSanitize, pArgsSanitize);
    if (!pResultSanitized) {
        PyErr_Print();
        fprintf(stderr, "Failed to call sanitize_info\n");
        Py_DECREF(pResult);
        Py_DECREF(pArgs);
        Py_DECREF(pMethod);
        Py_DECREF(pSanitize);
        Py_DECREF(pInstance);
        Py_DECREF(pDict);
        Py_DECREF(pClass);
        Py_DECREF(pModule);
        Py_Finalize();
        return NULL;
    }

    PyObject *pJsonModule = PyImport_ImportModule("json");
    if (!pJsonModule) {
        PyErr_Print();
        fprintf(stderr, "Failed to import json\n");
        Py_DECREF(pResultSanitized);
        Py_DECREF(pResult);
        Py_DECREF(pArgs);
        Py_DECREF(pMethod);
        Py_DECREF(pSanitize);
        Py_DECREF(pInstance);
        Py_DECREF(pDict);
        Py_DECREF(pClass);
        Py_DECREF(pModule);
        Py_Finalize();
        return NULL;
    }
    PyObject *pJsonDumps = PyObject_GetAttrString(pJsonModule, "dumps");
    if (!pJsonDumps) {
        PyErr_Print();
        fprintf(stderr, "Failed to get json.dumps\n");
        Py_DECREF(pResult);
        Py_DECREF(pResultSanitized);
        Py_DECREF(pArgs);
        Py_DECREF(pArgsSanitize);
        Py_DECREF(pMethod);
        Py_DECREF(pSanitize);
        Py_DECREF(pInstance);
        Py_DECREF(pDict);
        Py_DECREF(pClass);
        Py_DECREF(pJsonModule);
        Py_DECREF(pModule);
        Py_Finalize();
        return NULL;
    }

    // Call json.dumps(pResult, indent=2, ensure_ascii=False)
    PyObject *pJsonArgs = PyTuple_New(1);
    PyTuple_SetItem(pJsonArgs, 0, pResultSanitized); // Borrowed reference
    PyObject *pKwargs = PyDict_New();
    //PyDict_SetItemString(pKwargs, "indent", PyLong_FromLong(2));
    PyDict_SetItemString(pKwargs, "ensure_ascii", Py_False);
    PyObject *pRepr = NULL;
    PyObject *pJsonString = PyObject_Call(pJsonDumps, pJsonArgs, pKwargs);
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
        //Py_XDECREF(pRepr);
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
    Py_XDECREF(pJsonString);
    Py_DECREF(pKwargs);
    Py_DECREF(pJsonArgs);
    Py_DECREF(pJsonDumps);
    Py_DECREF(pResult);
    Py_DECREF(pResultSanitized);
    Py_DECREF(pArgs);
    Py_DECREF(pArgsSanitize);
    Py_DECREF(pMethod);
    Py_DECREF(pSanitize);
    Py_DECREF(pInstance);
    Py_DECREF(pDict);
    Py_DECREF(pClass);
    Py_DECREF(pJsonModule);
    Py_DECREF(pModule);
    Py_Finalize();
    return ret;
}