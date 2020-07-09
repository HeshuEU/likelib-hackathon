import signal
import subprocess
import time

from .base import Logger, LogicException, BadResultException


class Node:
    def __init__(self, *, name: str, work_dir: str, config_file_path: str, node_file_path: str, logger: Logger):
        self.name = name
        self.work_dir = work_dir
        self.config_file_path = config_file_path
        self.node_file_path = node_file_path
        self.logger = logger
        self.process = None
        self.is_running = False
        self.pid = -1

    def start(self, *, startup_time: int) -> None:
        if self.is_running:
            raise LogicException(f"{self.name} - Process already started")

        self.process = subprocess.Popen([self.node_file_path, "--config", self.config_file_path],
                                        cwd=self.work_dir, stderr=subprocess.DEVNULL, stdout=subprocess.DEVNULL)

        if self.process.poll() is None:
            self.is_running = True
            self.pid = self.process.pid
            self.logger.info(f"{self.name} - start node(pid:{self.pid}) with work directory: {self.work_dir}")
        else:
            self.is_running = False
            self.process.kill()
            self.logger.error(f"{self.name} - failed running node with work directory:{self.work_dir}")
            raise BadResultException(f"{self.name} - process failed to start")

        time.sleep(startup_time)

    def stop(self, *, shutdown_timeout: int) -> None:
        if self.is_running:
            self.logger.info(f"{self.name} - try to close node with work_dir {self.work_dir}")
            self.process.send_signal(signal.SIGINT)
            try:
                self.process.wait(timeout=shutdown_timeout)
            except subprocess.TimeoutExpired:
                self.process.kill()
                self.logger.info(f"{self.name} - killed node with work_dir {self.work_dir}")
            exit_code = self.process.poll()
            self.logger.info(
                f"{self.name} - closed node(exit code:{exit_code}, pid:{self.pid}, work_dir:{self.work_dir})")
            self.is_running = False
            self.pid = -1
