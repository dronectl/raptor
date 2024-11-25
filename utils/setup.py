import os
import subprocess
import logging
from pathlib import Path
from venv import EnvBuilder

VENV_DIR_NAME = "venv"

logging.basicConfig(format='%(message)s', level=logging.INFO)
_logger = logging.getLogger(__name__)

root = Path(__file__).parent.parent.resolve()
venv_path = root.joinpath(VENV_DIR_NAME).resolve()
req_path = root.joinpath('requirements.txt').resolve()

_logger.info("Building virtual environment")
env = EnvBuilder(clear=True,upgrade_deps=True, with_pip=True)
env.create(venv_path)
_logger.info("Building virtual environment: done")

_logger.info("Upgrading pip")
python_path = os.path.join(venv_path, "Scripts" if os.name == "nt" else "bin", "python")
pip_path = os.path.join(venv_path, "Scripts" if os.name == "nt" else "bin", "pip")
subprocess.run([python_path, "-m", "pip", "install", "--upgrade", "pip"], check=True)
_logger.info("Upgrading pip: done")
_logger.info("Installing third-party dependancies")
subprocess.run([pip_path, "install", "-r", str(req_path)], check=True)
_logger.info("Installing third-party dependancies: done")
_logger.info("Setup complete")

