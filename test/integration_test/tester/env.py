import os
import shutil


class Env:
    def __init__(self, binary_path):
        self.binary_path = binary_path
        self.client_path = self.check("client")
        self.encoder_path = self.check("encoder.py")
        self.decoder_path = self.check("decoder.py")
        self.node_path = self.check("node")
        self.evm_path = self.check("libevmone.so.0.4")

    def check(self, name) -> str:
        file_path = os.path.abspath(os.path.join(self.binary_path, name))
        if os.path.exists(file_path):
            return file_path
        else:
            raise Exception(f"file not found: {file_path}")

    def prepare(self, work_dir) -> None:
        if not os.path.exists(work_dir):
            os.makedirs(work_dir)

        shutil.copy(self.client_path, work_dir)
        shutil.copy(self.encoder_path, work_dir)
        shutil.copy(self.decoder_path, work_dir)
        shutil.copy(self.node_path, work_dir)
        shutil.copy(self.evm_path, work_dir)


DISTRIBUTOR_ADDRESS_PATH = os.path.realpath(
    os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..", "doc", "base-account-keys"))
