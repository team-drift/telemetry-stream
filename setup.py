from setuptools import setup, Extension
import pybind11
from pybind11.setup_helpers import Pybind11Extension, build_ext

ext_modules = [
    Pybind11Extension(
        "telemetry_bindings",
        ["telemetry_stream_async.cpp"],
        include_dirs=[
            pybind11.get_include(),
            "/extern/mavsdk",
            "/extern/nlohmannjson"
        ],
        library_dirs=[
            "/extern/mavsdk",
            "/extern/nlohmannjson"
        ],
        libraries=["mavsdk", "mavsdk_telemetry"],  # Add necessary libraries
        language="c++"
    ),
]

setup(
    name="telemetry_bindings",
    version="0.1",
    author="Swabhan Katkoori",
    author_email="sk@gmail.com",
    description="Python bindings for MAVSDK telemetry",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
)
