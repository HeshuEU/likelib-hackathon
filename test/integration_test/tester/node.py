import signal
import subprocess
import time

from .base import Logger


class Node:
    def __init__(self, *, name: str, work_dir: str, config_file_path: str, node_file_path: str, logger: Logger):
        self.name = name
        self.work_dir = work_dir
        self.config_file_path = config_file_path
        self.node_file_path = node_file_path
        self.logger = logger

        self.process = None
        self.is_running = False

    def start(self, *, startup_time: int) -> None:
        if self.is_running:
            raise Exception(f"{self.name} - Process already started")

        self.process = subprocess.Popen([self.node_file_path, "--config", self.config_file_path],
                                        cwd=self.work_dir, stderr=subprocess.DEVNULL, stdout=subprocess.DEVNULL)

        if self.process.poll() is None:
            self.logger.info(f"{self.name} - running node with work directory {self.work_dir}")
            self.is_running = True
        else:
            self.logger.info(f"{self.name} - failed running node with work directory:{self.work_dir}")
            self.is_running = False
            self.process.kill()
            raise Exception(f"{self.name} - Process failed to start")

        self.logger.debug(f"{self.name} - start node(pid:{self.pid}) with work directory: {self.work_dir}")

        time.sleep(startup_time)

    @property
    def pid(self) -> int:
        if self.is_running:
            return self.process.pid
        else:
            return -1

    def stop(self, *, shutdown_timeout: int) -> None:
        if self.is_running:
            pid = self.process.pid
            self.logger.info(f"{self.name} - try to close node with work_dir {self.work_dir}")
            self.process.send_signal(signal.SIGINT)
            try:
                self.process.wait(timeout=shutdown_timeout)
            except subprocess.TimeoutExpired:
                self.process.kill()
                self.logger.info(f"{self.name} - kill node with work_dir {self.work_dir}")
            exit_code = self.process.poll()
            self.is_running = False
            self.logger.info(f"{self.name} - closed node(exit code:{exit_code}, pid:{pid}, work_dir:{self.work_dir})")
