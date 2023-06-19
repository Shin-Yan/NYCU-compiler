#! /usr/bin/env python3
import sys
from os import environ as env
from string import Template
from configparser import ConfigParser
from datetime import datetime
import argparse

docker_env_tagname = "STATUS_DOCKER_ACTIVATED"

def get_args():
    parser = argparse.ArgumentParser(
        description='Activate homework environment for compiler-s20')
    parser.add_argument( '-r', '--restore',action='store_true',help="restore from template")
    parser.add_argument( '--config', default="student_info.ini", help="config file to use")
    parser.add_argument( '-i','--imagename', default='compiler-s20-env')
    return parser.parse_args()

def env_check():
    if "STATUS_DOCKER_ACTIVATED" not in env:
        print("Use ./activate_docker.sh to enter our docker environment first! ")
        sys.exit(0)

def main():
    args = get_args()
    restore_mode = args.restore

    config_filename = args.config
    config = ConfigParser()
    config.read(config_filename, encoding='utf-8')

    personal_info = dict(config['info'])
    if not restore_mode:
        personal_info['last_maketime'] = datetime.now().strftime("%Y-%m-%d %I:%M:%S %p")
        personal_info['docker_env_tag'] = env.get(docker_env_tagname, "NOT USING DOCKER")

    with open('./res/tmpl_readme.md',"r", encoding="utf-8" ) as tmpl:
        tmpl = Template(tmpl.read())
        final = tmpl.safe_substitute(personal_info)

    with open('README.md',"w",encoding='utf-8') as rm:
        rm.write(final )

if __name__ == "__main__":
    env_check()
    main()