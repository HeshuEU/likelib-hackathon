import os


def __import_test_case(case_name):
    try:
        imported_module = __import__(f"test_cases.{case_name}")
        main_fun = imported_module.__dict__[case_name].__dict__.get("main", None)
        return main_fun
    except Exception as e:
        print(f"Failed to import test case [{case_name}]: {e}")
        return None


def __register_test_cases(test_case_names):
    registered_test_cases = list()

    for test_case in test_case_names:
        if len(test_case) != 0:
            test_case_main_fun = __import_test_case(test_case);
            if test_case_main_fun is not None:
                content = {"name": test_case, "run": test_case_main_fun}
                registered_test_cases.append(content)

    return registered_test_cases


def __get_test_case_names():
    test_cases_dir = os.path.dirname(os.path.abspath(__file__))
    test_cases = list()
    for file_name in os.listdir(test_cases_dir):
        if file_name.endswith(".py") and file_name != "__init__.py":
            test_cases.append(os.path.splitext(file_name)[0])
    return test_cases


registered_test_cases = __register_test_cases(__get_test_case_names())

