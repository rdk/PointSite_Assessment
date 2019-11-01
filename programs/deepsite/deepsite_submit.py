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
#-> site_URL and login 
SITE_URL = 'https://playmolecule.com/deepsite'
USER = 'justest@000.com'
PASS = '123456'
#-> wait time
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
        self.driver.get(SITE_URL)
        self.login()
        LOGGER.info('login complete')
        form = self.find_el_by_css('#init-input-form', wait_second, self.driver)
        time.sleep(sleep_second)
        job_ids = self.get_all_jobs()
        top_job_id = job_ids[0]
        self.find_el_by_css('input[type=file]', wait_second, form).send_keys(self.input)
        time.sleep(sleep_second)
        LOGGER.info('send_file complete')
        time.sleep(sleep_second)
        submit_btn = self.find_el_by_css('button[type="submit"]', wait_second, form)
        if submit_btn.get_attribute('disabled') in ['disabled', 'true']:
            LOGGER.info('submit button is disabled. try select options to enable it.')
            select_el = self.find_el_by_css('md-select#form_select_chain_2', wait_second, form)
            select_el.click()
            time.sleep(sleep_second)
            first_option = self.find_el_by_css(
                '.md-select-menu-container.md-active.md-clickable md-option', wait_second, self.driver)
            all_option = self.find_el_by_css('md-option[value="all"]', wait_second, self.driver)
            if all_option:
                all_option.click()
                LOGGER.info('find all option. click it.')
            elif first_option:
                first_option.click()
                LOGGER.info('cannot find all option. click 1st option.')
            else:
                webdriver.ActionChains(self.driver).send_keys(Keys.ESCAPE).perform()
                raise Exception('submit button is disabled. cannot find any options to click.')
        LOGGER.info('submit button is enabled. move forward.')
        time.sleep(sleep_second)
        self.find_el_by_css('button[type="submit"]', wait_second, form).click()
        LOGGER.info('submit complete')
        time.sleep(sleep_second)
        while True:
            job_ids = self.get_all_jobs()
            # print(job_ids)
            if job_ids[0] == top_job_id:
                time.sleep(sleep_second)
            else:
                break
        new_job_id = job_ids[0]
        # print('new_job_id is '+new_job_id)
        self.start_download(new_job_id)

    def start_download(self, new_job_id):
        # self.driver.get(SITE_URL)
        # self.login()
        protein_id = os.path.split(self.input)[-1].split('.')[0]
        LOGGER.info('Job ID: {}'.format(new_job_id))
        LOGGER.info('Protein ID: {}'.format(protein_id))
        new_url = '{}/job/{}'.format(SITE_URL, new_job_id)
        self.driver.get(new_url)
        count = 1
        while True:
            time.sleep(refresh_second)
            side_bar = self.find_el_by_css('#widget-sidebar', wait_second, self.driver)
            csv_download_el = self.find_el_by_xpath('//*[contains(text(), \'Download results\')]', 3, side_bar)
            cub_download_el = self.find_el_by_xpath('//*[contains(text(), \'Download cube file\')]', 3, side_bar)
            if not (csv_download_el and cub_download_el):
                time.sleep(refresh_second)
                LOGGER.info('Result not ready. Refreshing.... Count: {}'.format(count))
                count += 1
                self.driver.refresh()
                if count >= maxwait_count:
                    LOGGER.info('ERROR: Reach maximal count: {}'.format(count))
                    sys.exit()
            else:
                csv_link = self.find_el_by_xpath('..', wait_second, csv_download_el).get_attribute('href')
                cub_link = self.find_el_by_xpath('..', wait_second, cub_download_el).get_attribute('href')
                break
        pdb_link = re.sub(re.escape('.csv') + "$", '.pdb', csv_link)
        self.download_file(csv_link, '{}.csv'.format(protein_id))
        self.download_file(pdb_link, '{}.pdb'.format(protein_id))
        self.download_file(cub_link, '{}.cube'.format(protein_id))

    def login(self):
        email = USER
        password = PASS
        time.sleep(sleep_second)
        menu_bar = self.find_el_by_css('#menu-bar-links', wait_second, self.driver)
        log_in_btn = self.find_el_by_xpath("//*[contains(text(), 'Log in')]", wait_second, menu_bar)
        log_in_btn.click()
        time.sleep(sleep_second)
        login_form = self.find_el_by_css('[name="loginForm"]', wait_second, self.driver)
        self.find_el_by_css('input[type="email"]', wait_second, login_form).send_keys(email)
        self.find_el_by_css('input[type="password"]', wait_second, login_form).send_keys(password)
        time.sleep(sleep_second)
        self.find_el_by_css('input#submit', wait_second, login_form).click()
        time.sleep(sleep_second)
        self.driver.refresh()
        time.sleep(sleep_second)

    def get_all_jobs(self):
        ids = []
        time.sleep(sleep_second)
        job_list = self.find_el_by_css('#job_list_container #job_list_table', wait_second, self.driver)
        time.sleep(sleep_second)
        job_id_els = self.find_els_by_css('tbody tr td:nth-child(2)', wait_second, job_list)
        time.sleep(sleep_second)
        for el in job_id_els:
            ids.append(el.text)
        return ids

    def download_file(self, url, file_name):
        r = requests.get(url)
        target = os.path.join(self.output, file_name)
        with open(target, 'wb') as f:
            f.write(r.content)
        LOGGER.info('Success: {}'.format(target))

    # find one element by xpath selector
    def find_el_by_xpath(self, selector, max_wait, root):
        max_wait = max_wait or maxwait_second
        wait = 0
        root = root or self.driver
        while True:
            els = root.find_elements_by_xpath(selector)
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
    parser = argparse.ArgumentParser(description='Process a PDB file on playmolecule site.')
    parser.add_argument('-i', action='store', dest='input', required=True, help='the absolute path for input pdb file')
    parser.add_argument('-o', action='store', dest='output', help='the output directory', default='./')
    parser.add_argument('-d', action='store', dest='chrom_drv', help='the chrome driver', default='./bin/chromedriver')
    parser.add_argument('-c', action='store', dest='config', help='config file for user and password', default='./config.json')
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
        # --- load user and password ----#
        with open(arg.config) as json_file:
            data = json.load(json_file)
            USER = data.get('email')
            PASS = data.get('password')
        # --- output arguments ----------#
        LOGGER.info('Args: chrom_drv='+arg.chrom_drv+',config='+arg.config+',sleep='+str(arg.sleep)+',refresh='+str(arg.refresh)+',wait='+str(arg.wait)+',maxwait='+str(arg.maxwait)+',maxcount='+str(arg.maxcount))
        # --- assign sleep_time and refresh_time ----#
        sleep_second = arg.sleep
        refresh_second = arg.refresh
        wait_second = arg.wait
        maxwait_second = arg.maxwait
        maxwait_count = arg.maxcount
        # --- run script ---#
        Spider().start()
