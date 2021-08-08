import argparse
import os
import shutil
import subprocess

def compile(a_args):
	compiler = os.path.join(a_args.f4dir, "Papyrus Compiler", "PapyrusCompiler.exe")
	vanilla = os.path.join(a_args.f4dir, "Data", "Scripts", "Source", "Base")
	output = os.getcwd()
	flags = "Institute_Papyrus_Flags.flg"

	for root, dirs, files in os.walk(os.path.join(a_args.src_dir, "src", "papyrus")):
		for file in files:
			if file.endswith(".psc"):
				pargs = [
					compiler,
					os.path.join(root, file),
					"-import={}".format(vanilla),
					"-output={}".format(output),
					"-flags={}".format(flags),
				]
				if a_args.debug == "false":
					pargs.append("-optimize")
					pargs.append("-release")
					pargs.append("-final")
				subprocess.run(pargs, check=True)

	if a_args.copy_build == "true":
		for root, dirs, files in os.walk(output):
			for file in files:
				if file.endswith(".pex"):
					shutil.copy(file, os.path.join(a_args.f4dir, "Data", "Scripts"))

def parse_arguments():
	parser = argparse.ArgumentParser(description="compile papyrus source scripts")
	parser.add_argument("--copy-build", type=str, help="copy the compiled artifacts into the fallout 4 directory", required=True)
	parser.add_argument("--debug", type=str, help="enable papyrus debugging commands", required=True)
	parser.add_argument("--f4dir", type=str, help="the fallout 4 directory", required=True)
	parser.add_argument("--src-dir", type=str, help="the project root source directory", required=True)
	return parser.parse_args()

def main():
	out = "artifacts"
	try:
		os.mkdir(out)
	except FileExistsError:
		pass
	os.chdir(out)

	args = parse_arguments()
	compile(args)

if __name__ == "__main__":
	main()
