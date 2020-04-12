from setuptools import setup, find_packages
from Cython.Build import cythonize

setup(
    name='skipgraph',
    version='0.1.0',
    author='Mark Bell',
    author_email='mcbell@illinois.edu',
    license='MIT License',
    packages=find_packages(),
    ext_modules=cythonize([
        './skipgraph/skipgraph.pyx',
        ], annotate=True, language_level=3),
    description = 'A graph library which implements the ball skip algorithm to compute diameters quickly.',
    )

