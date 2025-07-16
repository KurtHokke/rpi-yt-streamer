#include <python3.13/Python.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    // Initialize the Python interpreter
    Py_Initialize();

    // Add current directory to sys.path to ensure yt_dlp is found
    PyRun_SimpleString("import sys\nsys.path.append('./3rd/yt-dlp')\n");
    PyRun_SimpleString("import sys\nsys.path.append('/home/src/c/rpi-yt-streamer/bin/_venv/lib/python3.13/site-packages')\n");

    // Import yt_dlp module
    PyObject *pModule = PyImport_ImportModule("yt_dlp");
    if (!pModule) {
        PyErr_Print();
        fprintf(stderr, "Failed to import yt_dlp\n");
        Py_Finalize();
        return 1;
    }
    // Get YoutubeDL class
    PyObject *pClass = PyObject_GetAttrString(pModule, "YoutubeDL");
    if (!pClass) {
        PyErr_Print();
        fprintf(stderr, "Failed to get YoutubeDL class\n");
        Py_DECREF(pModule);
        Py_Finalize();
        return 1;
    }
    // Import json module for serialization
    PyObject *pJsonModule = PyImport_ImportModule("json");
    if (!pJsonModule) {
        PyErr_Print();
        fprintf(stderr, "Failed to import json\n");
        Py_DECREF(pClass);
        Py_DECREF(pModule);
        Py_Finalize();
        return 1;
    }

    // Create options dictionary
    PyObject *pDict = PyDict_New();
    if (!pDict) {
        PyErr_Print();
        fprintf(stderr, "Failed to create options dictionary\n");
        Py_DECREF(pClass);
        Py_DECREF(pJsonModule);
        Py_DECREF(pModule);
        Py_Finalize();
        return 1;
    }
    PyDict_SetItemString(pDict, "extract_flat", PyUnicode_FromString("discard_in_playlist"));
    PyDict_SetItemString(pDict, "format", PyUnicode_FromString("bestvideo[height<=1080]+bestaudio/best[height<=1080]"));
    PyDict_SetItemString(pDict, "fragment_retries", PyLong_FromLong(10));
    PyDict_SetItemString(pDict, "retries", PyLong_FromLong(10));
    PyDict_SetItemString(pDict, "ignoreerrors", PyUnicode_FromString("only_download"));
    //PyDict_SetItemString(pDict, "listsubtitles", Py_True);
    // Optional: Specify FFmpeg path for aarch64
    PyDict_SetItemString(pDict, "ffmpeg_location", PyUnicode_FromString("/home/src/c/rpi-yt-streamer/bin/ffmpeg"));

    PyObject *pPostprocessorsList = PyList_New(1);
    if (!pPostprocessorsList) {
        PyErr_Print();
        fprintf(stderr, "Failed to create postprocessors list\n");
        Py_DECREF(pDict);
        Py_DECREF(pClass);
        Py_DECREF(pJsonModule);
        Py_DECREF(pModule);
        Py_Finalize();
        return 1;
    }
    PyObject *pPostprocessorDict = PyDict_New();
    if (!pPostprocessorDict) {
        PyErr_Print();
        fprintf(stderr, "Failed to create postprocessor dictionary\n");
        Py_DECREF(pPostprocessorsList);
        Py_DECREF(pDict);
        Py_DECREF(pClass);
        Py_DECREF(pJsonModule);
        Py_DECREF(pModule);
        Py_Finalize();
        return 1;
    }
    PyDict_SetItemString(pPostprocessorDict, "key", PyUnicode_FromString("FFmpegConcat"));
    PyDict_SetItemString(pPostprocessorDict, "only_multi_video", Py_True);
    PyDict_SetItemString(pPostprocessorDict, "when", PyUnicode_FromString("playlist"));

    // Add postprocessor dictionary to list
    PyList_SetItem(pPostprocessorsList, 0, pPostprocessorDict); // Takes ownership of pPostprocessorDict

    // Set postprocessors list in main dictionary
    PyDict_SetItemString(pDict, "postprocessors", pPostprocessorsList);

    // Instantiate YoutubeDL
    PyObject *pInstance = PyObject_CallObject(pClass, PyTuple_Pack(1, pDict));
    if (!pInstance) {
        PyErr_Print();
        fprintf(stderr, "Failed to instantiate YoutubeDL\n");
        Py_DECREF(pPostprocessorsList);
        Py_DECREF(pDict);
        Py_DECREF(pClass);
        Py_DECREF(pModule);
        Py_DECREF(pJsonModule);
        Py_Finalize();
        return 1;
    }

    // Prepare URL list (use a playlist URL to trigger FFmpegConcat)
    //PyObject *pUrlList = PyList_New(1);
    //PyList_SetItem(pUrlList, 0, PyUnicode_FromString("https://www.youtube.com/watch?v=sdqfcfq5aM4"));

    // Get extract_info method
    PyObject *pMethod = PyObject_GetAttrString(pInstance, "extract_info");
    if (!pMethod) {
        PyErr_Print();
        fprintf(stderr, "Failed to get extract_info method\n");
        Py_DECREF(pInstance);
        Py_DECREF(pPostprocessorsList);
        Py_DECREF(pDict);
        Py_DECREF(pClass);
        Py_DECREF(pModule);
        Py_DECREF(pJsonModule);
        Py_Finalize();
        return 1;
    }
    PyObject *pMethodSanitize = PyObject_GetAttrString(pInstance, "sanitize_info");
    if (!pMethodSanitize) {
        PyErr_Print();
        fprintf(stderr, "Failed to get sanitize_info method\n");
        Py_DECREF(pMethod);
        Py_DECREF(pInstance);
        Py_DECREF(pPostprocessorsList);
        Py_DECREF(pDict);
        Py_DECREF(pClass);
        Py_DECREF(pModule);
        Py_DECREF(pJsonModule);
        Py_Finalize();
        return 1;
    }

    // Prepare arguments for extract_info(url, download=False, ie_key=None, extra_info=None, process=True, force_generic_extractor=False)
    PyObject *pArgs = PyTuple_New(6);
    PyTuple_SetItem(pArgs, 0, PyUnicode_FromString("https://www.youtube.com/watch?v=sdqfcfq5aM4")); // url
    PyTuple_SetItem(pArgs, 1, Py_True); // download=True
    PyTuple_SetItem(pArgs, 2, Py_None);  // ie_key=None
    PyTuple_SetItem(pArgs, 3, Py_None);  // extra_info=None
    PyTuple_SetItem(pArgs, 4, Py_True);  // process=True
    PyTuple_SetItem(pArgs, 5, Py_False); // force_generic_extractor=False

    // Call extract_info
    PyObject *pResult = PyObject_CallObject(pMethod, pArgs);
    if (!pResult) {
        PyErr_Print();
        fprintf(stderr, "Failed to call extract_info\n");
        Py_DECREF(pArgs);
        Py_DECREF(pMethod);
        Py_DECREF(pMethodSanitize);
        Py_DECREF(pInstance);
        Py_DECREF(pPostprocessorsList);
        Py_DECREF(pDict);
        Py_DECREF(pClass);
        Py_DECREF(pJsonModule);
        Py_DECREF(pModule);
        Py_Finalize();
        return 1;
    }
    PyObject *pArgsSanitize = PyTuple_New(1);
    PyTuple_SetItem(pArgsSanitize, 0, pResult);
    PyObject *pResultSanitized = PyObject_CallObject(pMethodSanitize, pArgsSanitize);
    if (!pResultSanitized) {
        PyErr_Print();
        fprintf(stderr, "Failed to call sanitize_info\n");
        Py_DECREF(pResult);
        Py_DECREF(pArgs);
        Py_DECREF(pMethod);
        Py_DECREF(pMethodSanitize);
        Py_DECREF(pInstance);
        Py_DECREF(pPostprocessorsList);
        Py_DECREF(pDict);
        Py_DECREF(pClass);
        Py_DECREF(pJsonModule);
        Py_DECREF(pModule);
        Py_Finalize();
        return 1;
    }
    
    // Convert pResult to JSON string
    PyObject *pJsonDumps = PyObject_GetAttrString(pJsonModule, "dumps");
    if (!pJsonDumps) {
        PyErr_Print();
        fprintf(stderr, "Failed to get json.dumps\n");
        Py_DECREF(pResult);
        Py_DECREF(pResultSanitized);
        Py_DECREF(pArgs);
        Py_DECREF(pArgsSanitize);
        Py_DECREF(pMethod);
        Py_DECREF(pMethodSanitize);
        Py_DECREF(pInstance);
        Py_DECREF(pPostprocessorsList);
        Py_DECREF(pDict);
        Py_DECREF(pClass);
        Py_DECREF(pJsonModule);
        Py_DECREF(pModule);
        Py_Finalize();
        return 1;
    }

    // Call json.dumps(pResult, indent=2, ensure_ascii=False)
    PyObject *pJsonArgs = PyTuple_New(1);
    PyTuple_SetItem(pJsonArgs, 0, pResultSanitized); // Borrowed reference
    PyObject *pKwargs = PyDict_New();
    PyDict_SetItemString(pKwargs, "indent", PyLong_FromLong(2));
    PyDict_SetItemString(pKwargs, "ensure_ascii", Py_False);
    PyObject *pJsonString = PyObject_Call(pJsonDumps, pJsonArgs, pKwargs);
    if (!pJsonString || !PyUnicode_Check(pJsonString)) {
        PyErr_Print();
        fprintf(stderr, "Failed to serialize pResult to JSON\n");
        // Fallback to repr()
        PyObject *pRepr = PyObject_Repr(pResult);
        if (pRepr && PyUnicode_Check(pRepr)) {
            printf("pResult (repr):\n%s\n", PyUnicode_AsUTF8(pRepr));
        }
        Py_XDECREF(pRepr);
    } else {
        printf("pResult (JSON):\n%s\n", PyUnicode_AsUTF8(pJsonString));
    }

    // Clean up
    Py_XDECREF(pJsonString);
    Py_DECREF(pKwargs);
    Py_DECREF(pJsonArgs);
    Py_DECREF(pJsonDumps);
    Py_DECREF(pResult);
    Py_DECREF(pResultSanitized);
    Py_DECREF(pArgs);
    Py_DECREF(pArgsSanitize);
    Py_DECREF(pMethod);
    Py_DECREF(pMethodSanitize);
    Py_DECREF(pInstance);
    Py_DECREF(pPostprocessorsList);
    Py_DECREF(pDict);
    Py_DECREF(pClass);
    Py_DECREF(pJsonModule);
    Py_DECREF(pModule);
    Py_Finalize();

    return 0;
}
/*
    // Extract metadata (e.g., title, uploader, duration)
    PyObject *pTitle = PyDict_GetItemString(pResult, "title");
    PyObject *pUploader = PyDict_GetItemString(pResult, "uploader");
    PyObject *pDuration = PyDict_GetItemString(pResult, "duration");
    PyDict_Get
    if (pTitle && PyUnicode_Check(pTitle)) {
        printf("Title: %s\n", PyUnicode_AsUTF8(pTitle));
    }
    if (pUploader && PyUnicode_Check(pUploader)) {
        printf("Uploader: %s\n", PyUnicode_AsUTF8(pUploader));
    }
    if (pDuration && PyFloat_Check(pDuration)) {
        printf("Duration: %.2f seconds\n", PyFloat_AsDouble(pDuration));
    } else if (pDuration && PyLong_Check(pDuration)) {
        printf("Duration: %ld seconds\n", PyLong_AsLong(pDuration));
    }

    // Clean up
    Py_DECREF(pArgs);
    Py_DECREF(pMethod);
    Py_DECREF(pResult);
    Py_DECREF(pInstance);
    Py_DECREF(pPostprocessorsList);
    Py_DECREF(pDict);
    Py_DECREF(pClass);
    Py_DECREF(pModule);
    Py_DECREF(pJsonModule);
    Py_Finalize();

    return 0;
}*/
/*
    // Call download method
    PyObject *pMethod = PyObject_GetAttrString(pInstance, "download");
    if (!pMethod) {
        PyErr_Print();
        fprintf(stderr, "Failed to get download method\n");
        Py_DECREF(pUrlList);
        Py_DECREF(pInstance);
        Py_DECREF(pPostprocessorsList);
        Py_DECREF(pDict);
        Py_DECREF(pClass);
        Py_DECREF(pModule);
        Py_Finalize();
        return 1;
    }

    PyObject *pResult = PyObject_CallObject(pMethod, PyTuple_Pack(1, pUrlList));
    if (!pResult) {
        PyErr_Print();
        fprintf(stderr, "Download failed\n");
    } else {
        printf("Download and concatenation completed successfully\n");
    }

    // Clean up
    Py_XDECREF(pResult);
    Py_DECREF(pMethod);
    Py_DECREF(pUrlList);
    Py_DECREF(pInstance);
    Py_DECREF(pPostprocessorsList);
    Py_DECREF(pDict);
    Py_DECREF(pClass);
    Py_DECREF(pModule);
    Py_Finalize();

    return 0;
}*/