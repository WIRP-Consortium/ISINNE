import re
import secrets
import datetime
from dateutil.relativedelta import relativedelta
from pathlib import Path
import ast
from cryptography.hazmat.primitives.serialization import load_pem_private_key
from cryptography.hazmat.primitives import hashes
import socket
from datetime import datetime
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives import serialization

FILE = Path("Network Names.log")

def read_registration(filename):
    data = Path(filename).read_text(encoding="utf-8")

    result = {
        "registrar": None,
        "fields": {},
        "verification": None
    }

    match = re.search(r"^\$REGISTRAR\s+(.+)$", data, re.M)
    if match:
        result["registrar"] = match.group(1).strip()

    match = re.search(
        r"@VERIFIED BY (.*?), EXPIRES IN (.*?)@",
        data
    )
    if match:
        result["verification"] = {
            "verified_by": match.group(1).strip(),
            "expires": match.group(2).strip()
        }

    pattern = re.compile(
        r"^\s*([@!?#]?)\s*([A-Z]+)\s+(.*?)\s*$",
        re.MULTILINE
    )

    lines = data.splitlines()

    i = 0
    while i < len(lines):
        line = lines[i].strip()

        match = re.match(
            r"^([@!?#]?)\s*([A-Z]+)\s+(.*)$",
            line
        )

        if not match:
            i += 1
            continue

        prefix, key, value = match.groups()

        if value.startswith("b'"):
            while not value.endswith("'") or value.count("\\n") < 2:
                i += 1
                if i >= len(lines):
                    break
                value += "\\n" + lines[i].strip()

        result["fields"][key] = {
            "prefix": prefix,
            "value": value.strip()
        }

        i += 1

    return result


def display_registration(reg):

    print("Registrar:", reg["registrar"])


    for key, item in reg["fields"].items():
        value = item["value"]

        if key in ("SECKEY", "SECKEYH"):
            value = "[SENSITIVE DATA HIDDEN]"

        print(f"{key:10} : {value}")


    print(reg["verification"])



def register():
    print("wddd or www")
    internet = input("Enter Internet type: ").strip()
    if internet == "wddd":
        name = "idtp/::wddd:"
        var = "wddd "
    elif internet == "www":
        name = "www."
        var = "www  "
    else:
        print("Invalid Statement!")
        return
    
    tname = input("Enter Name: ")
    tname = tname.lower().replace(" ", "")
    tname = re.sub(r'[^a-z0-9]', '', tname)

    name = f"{name}{tname}"

    print("""
1 = -rev
2 = -rrn
3 = -edu
4 = -prcn
5 = -prc
""")
    extension = int(input("Enter Extension: "))

    if extension == 1:
        name = f"{name}-rev"
        tname = f"{tname}-rev"
        id = secrets.token_urlsafe(6)[:6]
        id = f"{id}-REV-TCHR"
        root_server = "rev-thrichur-gsl"
        root_ip = "192.168.1.8"
    elif extension == 2:
        print("""YOUR LEGAL AGREEMENT""")
        agree = input("Agree with it [y/n]: ")
        if agree == "y":
            print("Continue Registration!")
        elif agree == "n":
            print("You can't take imperian name!!!")
            return
        else:
            print("Invalid!")
            return
        
        name = f"{name}-rrn"
        tname = f"{tname}-rrn"
        id = secrets.token_urlsafe(6)[:6]
        id = f"{id}-RRN-TCHR"
        root_server = "rrn-thrichur-gsl"
        root_ip = "192.168.1.8"
    elif extension == 3:
        name = f"{name}-edu"
        tname = f"{tname}-edu"
        root_server = "edu-ginna-gsl"
        root_ip = "103.145.172.173"
        id = secrets.token_urlsafe(6)[:6]
        id = f"{id}-EDU-GINA"
    elif extension == 4:
        name = f"{name}-prcn"
        tname = f"{tname}-prcn"
        root_server = "prcn-ginna-gsl"
        root_ip = "103.145.172.173"
        id = secrets.token_urlsafe(6)[:6]
        id = f"{id}-EDU-GINA"
    elif extension == 5:
        name = f"{name}-edu"
        tname = f"{tname}-edu"
        root_server = "edu-neumann-gsl"
        root_ip = "103.145.172.173"
        id = secrets.token_urlsafe(6)[:6]
        id = f"{id}-EDU-GINA"
    else:
        print("Invalid Number!")
        return

    ip = input("Enter IP V4: ").strip()
    ipv6 = input("Enter IP V6: ").strip()

    print("""
1 - 1 year
2 - 2 year
3 - 4 year
4 - 8 year
5 - 10 year
            """)
    expiry = int(input("Enter Expiry: "))

    txt = input("Your Text for your Imperian: ")

    email = input("Enter Email for Imperian: ")
    if email is None:
        email = ""
    else:
        email = f"{email}@{tname}"

    company = input("Enter Company Name: ")
    countryc = input("Country Code: ")
    phonenumber = input("Enter Contact Number: ")
    province_state = input("Enter Province / State: ")
    city = input("Enter City: ")
    street =  input("Enter Street: ")
    address = input("Address: ")
    postalcode = input("Enter Postal Code: ")
    fax = input("Enter Fax: ")
    techadmin = input("Enter Tech Admin: ")

    now = datetime.now()
    if expiry == 1:
        expiry = now + relativedelta(years=1)
    elif expiry == 2:
        expiry = now + relativedelta(years=2)
    elif expiry == 3:
        expiry = now + relativedelta(years=4)
    elif expiry == 4:
        expiry = now + relativedelta(years=8)
    elif expiry == 5:
        expiry = now + relativedelta(years=10)
    else:
        print("Invalid Number!")
        return

    file = read_registration(FILE)
    fields = file["fields"]

    registrar_name = fields["N"]["value"]
    registrar_id = fields["GID"]["value"]

    private_key_bytes = ast.literal_eval(
        fields["SECKEY"]["value"]
    )
    private_key = load_pem_private_key(
        private_key_bytes,
        password=None
    )

    public_key_pem = private_key.public_key().public_bytes(
        encoding=serialization.Encoding.PEM,
        format=serialization.PublicFormat.SubjectPublicKeyInfo
    )


    signature = private_key.sign(
        name.encode("utf-8"),
        ec.ECDSA(hashes.SHA256())
    )

    signature_hex = signature.hex()

    request = (f"""
NAME: {tname}
IMPERIAN: {name}
ID: {id}
IPV4: {ip}
IPV6: {ipv6}
EMAIL: {email}
TXT: {txt}
COMPANY: {company}
ISO: {countryc}
PHONE: {phonenumber}
PROVINCE/STATE: {province_state}
CITY: {city}
STREET: {street}
ADDRESS: {address}
POSTAL: {postalcode}
FAX: {fax}
ADMIN: {techadmin}
REGISTRAR: {registrar_name}
REGISTRAR ID: {registrar_id}
CREATION: {now}
EXPIRY: {expiry}
SIGNATURE: {signature_hex}
""")

    GINNA_IP = '192.168.1.8'
    PORT = 23
    ginna = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        ginna.connect((GINNA_IP, PORT))
        ginna.send(request.encode('utf-8'))
        ginna.shutdown(socket.SHUT_WR)
        print("Requested!!!")
    except ConnectionRefusedError:
        print("ERROR")
    finally:
        ginna.close()

def keys():
    file = read_registration(FILE)
    fields = file["fields"]

    registrar_name = fields["N"]["value"]
    registrar_id = fields["GID"]["value"]

    private_key_bytes = ast.literal_eval(
        fields["SECKEY"]["value"]
    )

    private_key = load_pem_private_key(
        private_key_bytes,
        password=None
    )
    print(private_key)
    print(private_key_bytes)
    
def main():
    print("GINNA Certified Registrar")
    while True:
        file = read_registration(FILE)
        cmd = input("GCR@ISINNE~$ ")
        if cmd == "register":
            register()
        elif cmd == "read":
            file = read_registration(FILE)
            print(file)
        elif cmd == "exit":
            return
        elif cmd == "key":
            key = keys()
        else:
            print("Read Registration")


if __name__ == "__main__":
    main()
