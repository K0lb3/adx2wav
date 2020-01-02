import os
from setuptools import Extension, setup

try:
	from Cython.Build import cythonize
except ImportError:
	cythonize = None



extensions = [
	Extension(
		name="adx2wav",
		version="1.0.0",
		author="K0lb3",
		author_email="",
		description="converts ADX to WAV",
		long_description=open('README.md', 'rt', encoding='utf8').read(),
		long_description_content_type="text/markdown",
		url="https://github.com/K0lb3/adx2wav",
		download_url="https://github.com/K0lb3/adx2wav/tarball/master",
		keywords=["adx","wav","usm","sound","conversion","audio","criware"],
		classifiers=[
			"Development Status :: 3 - Alpha",
			"Intended Audience :: Developers",
			"Programming Language :: Python",
			"Programming Language :: Python :: 3",
			"Programming Language :: Python :: 3.4",
			"Programming Language :: Python :: 3.5",
			"Programming Language :: Python :: 3.6",
			"Programming Language :: Python :: 3.7",
			"Programming Language :: Python :: 3.8",
			"Programming Language :: Python :: 3.9",
			"Topic :: Multimedia :: Sound/Audio :: Conversion"
		],
		sources=[
			'adx2wav.c',
		],
		language="c",
	)
]
if cythonize:
	extensions = cythonize(extensions)

setup(ext_modules=extensions)
