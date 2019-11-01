import time
import os
import re
import requests
import argparse
import sys
import json
import logging

from selenium import webdriver
from selenium.webdriver.chrome.options import Options
from selenium.webdriver.common.keys import Keys

# ------ global parameters ------#
#-> logger printf
logging.basicConfig(
    format='%(asctime)s %(levelname)-8s %(message)s',
    level=logging.INFO,
    datefmt='%Y-%m-%d %H:%M:%S')
LOGGER = logging.getLogger('mole')
#-> site_URL
SITE_URL = 'https://projects.biotec.tu-dresden.de/metapocket'
#-> wait time
ligand_number = 10
sleep_second = 2
refresh_second = 5
wait_second = 20
maxwait_second = 60
maxwait_count = 100


# ------- class definition ------#
class Spider(object):
    def __init__(self):
        self.input = arg.input
        self.output = arg.output
        LOGGER.info('Input: {}'.format(self.input))
        LOGGER.info('Output: {}'.format(self.output))
        chrome_options = Options()
        chrome_options.add_argument("--headless")
        self.driver = webdriver.Chrome(executable_path=arg.chrom_drv, options=chrome_options)
        self.driver.set_window_size(2000, 1300)

    def start(self):
        #-> get protein_id
        protein_id = os.path.split(self.input)[-1].split('.')[0]
        #-> goto the URL
        self.driver.get(SITE_URL)
        time.sleep(sleep_second)
        #-> change maximal ligand number
        lignum_str = '#page-content > table > tbody > tr > td:nth-child(1) > form > fieldset > table > tbody > tr > td > table > tbody > tr:nth-child(7) > td:nth-child(2) > input[type=text]'
        self.find_el_by_css(lignum_str, wait_second, self.driver).clear()
        self.find_el_by_css(lignum_str, wait_second, self.driver).send_keys(ligand_number)
        LOGGER.info('Lignum changed')
        time.sleep(sleep_second)
        #-> upload file
        upload_str = '#page-content > table > tbody > tr > td:nth-child(1) > form > fieldset > table > tbody > tr > td > table > tbody > tr:nth-child(5) > td:nth-child(2) > input[type=file]'
        self.find_el_by_css(upload_str, wait_second, self.driver).send_keys(self.input)
        LOGGER.info('file uploaded')
        time.sleep(sleep_second)
        #-> click submit button
        button_str = '#page-content > table > tbody > tr > td:nth-child(1) > form > fieldset > table > tbody > tr > td > table > tbody > tr:nth-child(11) > td > input[type=SUBMIT]:nth-child(1)'
        self.find_el_by_css(button_str, wait_second, self.driver).click()
        LOGGER.info('button clicked')
        time.sleep(sleep_second)
        #-> wait to download
        cent_result_str = '#page-content > div:nth-child(2) > table > tbody > tr:nth-child(8) > td > table > tbody > tr:nth-child(5) > td > a'
        surf_result_str = '#page-content > div:nth-child(2) > table > tbody > tr:nth-child(8) > td > table > tbody > tr:nth-child(7) > td > a'
        count = 1
        while True:
            time.sleep(refresh_second)
            cent_result = self.driver.find_elements_by_css_selector(cent_result_str)
            surf_result = self.driver.find_elements_by_css_selector(surf_result_str)
            if not (cent_result and surf_result):
                time.sleep(refresh_second)
                LOGGER.info('Result not ready. Refreshing.... Count: {}'.format(count))
                count += 1
                self.driver.refresh()
                if count >= maxwait_count:
                    LOGGER.info('ERROR: Reach maximal count: {}'.format(count))
                    sys.exit()
            else:
                time.sleep(sleep_second)
                cent_link_ = self.find_el_by_css(cent_result_str, wait_second, self.driver).get_attribute('href')
                cent_link = re.sub("^"+re.escape('http'), 'https', cent_link_)
                LOGGER.info('cent_link: {}'.format(cent_link))
                surf_link_ = self.find_el_by_css(surf_result_str, wait_second, self.driver).get_attribute('href')
                surf_link = re.sub("^"+re.escape('http'), 'https', surf_link_)
                LOGGER.info('surf_link: {}'.format(surf_link))
                break
        self.download_file(cent_link, '{}.cent'.format(protein_id))
        self.download_file(surf_link, '{}.surf'.format(protein_id))

    def download_file(self, url, file_name):
        r = requests.get(url)
        target = os.path.join(self.output, file_name)
        with open(target, 'wb') as f:
            f.write(r.content)
        LOGGER.info('Success: {}'.format(target))

    # find one element by css selector
    def find_el_by_css(self, selector, max_wait, root):
        max_wait = max_wait or maxwait_second
        wait = 0
        root = root or self.driver
        while True:
            els = root.find_elements_by_css_selector(selector)
            if len(els) == 0:
                time.sleep(1)
                wait += 1
            else:
                for el in els:
                    root_doms = el.find_elements_by_xpath(".//ancestor::body")
                    if len(root_doms) > 0:
                        return el
                time.sleep(1)
                wait += 1
            if wait > max_wait:
                return None
            else:
                LOGGER.info('cannot find element {}, keep waiting: {}/{}'.format(selector, wait, max_wait))

    # find multiple elements by css selector
    def find_els_by_css(self, selector, max_wait, root):
        res = []
        max_wait = max_wait or maxwait_second
        wait = 0
        root = root or self.driver
        while True:
            els = root.find_elements_by_css_selector(selector)
            if len(els) == 0:
                time.sleep(1)
                wait += 1
            else:
                for el in els:
                    root_doms = el.find_elements_by_xpath(".//ancestor::body")
                    if len(root_doms) > 0:
                        res.append(el)
                if len(res) > 0:
                    return res
                time.sleep(1)
                wait += 1
            if wait > max_wait:
                return []
            else:
                LOGGER.info('cannot find element {}, keep waiting: {}/{}'.format(selector, wait, max_wait))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Process a PDB file on metapocket2 site.')
    parser.add_argument('-i', action='store', dest='input', required=True, help='the absolute path for input pdb file')
    parser.add_argument('-o', action='store', dest='output', help='the output directory', default='./')
    parser.add_argument('-d', action='store', dest='chrom_drv', help='the chrome driver', default='./bin/chromedriver')
    parser.add_argument('-n', action='store', dest='lignum', type=int, help='the maximal number of ligand', default=10)
    parser.add_argument('-s', action='store', dest='sleep', type=int, help='sleep time, such as 2', default=2)
    parser.add_argument('-r', action='store', dest='refresh', type=int, help='refresh time, such as 5', default=5)
    parser.add_argument('-w', action='store', dest='wait', type=int, help='wait time, such as 20', default=20)
    parser.add_argument('-m', action='store', dest='maxwait', type=int, help='maximal wait time, such as 20', default=60)
    parser.add_argument('-M', action='store', dest='maxcount', type=int, help='maximal refresh count, such as 100', default=100)
    arg = parser.parse_args()
    LOGGER.info('Running Script: {}'.format(str(sys.argv[0])))
    if '-h' in sys.argv or '--help' in sys.argv:
        LOGGER.info(args.echo)
    else:
        # --- output arguments ----------#
        LOGGER.info('Args: chrom_drv='+arg.chrom_drv+',lignum='+str(arg.lignum)+',sleep='+str(arg.sleep)+',refresh='+str(arg.refresh)+',wait='+str(arg.wait)+',maxwait='+str(arg.maxwait)+',maxcount='+str(arg.maxcount))
        ligand_number = arg.lignum
        # --- assign sleep_time and refresh_time ----#
        sleep_second = arg.sleep
        refresh_second = arg.refresh
        wait_second = arg.wait
        maxwait_second = arg.maxwait
        maxwait_count = arg.maxcount
        # --- run script ---#
        Spider().start()

