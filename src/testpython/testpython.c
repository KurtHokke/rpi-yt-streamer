#include <Python.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    Py_Initialize();

    // Run Python code with threading
    const char *script =
        "import threading\n"
        "import time\n"
        "def worker(n):\n"
        "    for _ in range(1000000):\n"
        "        pass\n"
        "    print(f'Thread {n} (ID: {threading.get_ident()}) done')\n"
        "threads = [threading.Thread(target=worker, args=(i,)) for i in range(4)]\n"
        "start = time.time()\n"
        "for t in threads: t.start()\n"
        "for t in threads: t.join()\n"
        "print(f'Total time: {time.time() - start} seconds')\n";

    PyRun_SimpleString(script);

    Py_Finalize();
    return 0;
}