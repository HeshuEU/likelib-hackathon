import logging
import os


class TimeOutException(Exception):
    pass


class BadResultException(Exception):
    pass


class CheckFailedException(Exception):
    pass


class InvalidArgumentsException(Exception):
    pass


class LogicException(Exception):
    pass


# logger
class Logger:
    LOG_FORMAT = '%(asctime)s - %(levelname)s - %(message)s'

    def __init__(self, log_name, file_path, log_level=logging.DEBUG):
        self.logger = logging.getLogger(log_name)
        self.logger.setLevel(log_level)

        fh = logging.FileHandler(os.path.abspath(file_path))
        fh.setLevel(log_level)

        formatter = logging.Formatter(Logger.LOG_FORMAT)
        fh.setFormatter(formatter)

        self.logger.addHandler(fh)

    def info(self, message) -> None:
        self.logger.info(message)
        self.flush()

    def debug(self, message) -> None:
        self.logger.debug(message)
        self.flush()

    def error(self, message) -> None:
        self.logger.error(message)
        self.flush()

    def flush(self) -> None:
        for handler in self.logger.handlers:
            handler.flush()
