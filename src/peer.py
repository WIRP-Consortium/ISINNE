import hashlib
import secrets
import sys
import os
import re
import subprocess
import ipaddress
import hmac
import requests
import psutil
import time
import socket
import threading
import ssl
from datetime import datetime, timedelta
import pydoc

from cryptography.hazmat.primitives.asymmetric import ed25519
from cryptography.hazmat.primitives.serialization import (
    Encoding,
    PublicFormat
)
from cryptography.fernet import Fernet
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import ec
from Crypto.PublicKey import RSA

from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives import hashes, serialization
from dateutil.relativedelta import relativedelta


from cryptography.exceptions import InvalidSignature

from datetime import datetime, date
from pathlib import Path

PORT = 53
server_process = None

DATA_DIR = "data"
LOG_DIR = "log"
CHAIN_FILE = os.path.join(DATA_DIR, "name.conf")
REGISTRAR = os.path.join(DATA_DIR, "registrar.conf")

def create_tls_connection(root_ip, port):
    context = ssl.create_default_context(
        cafile="root_ca.crt"
    )

    return context.wrap_socket(
        socket.create_connection(
            (root_ip, port),
            timeout=10
        ),
        server_hostname=root_ip
    )

def garnn():
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
    tname = input("Enter Imperian Name: ").strip()
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
        root_ip = "IP"
    elif extension == 2:
        print("""
    ARTICLE IX: REGISTRY SERVICE COMPLIANCE, DEFAULT, AND THE ABSOLUTE RECLAMATION PROTOCOL ("KILL SWITCH")

    Section 9.01 – Scope and Autonomous Enforcement Authority

    The Registrant explicitly acknowledges and agrees that the .rrn Top-Level Zone
    operates as a centralized, heavily regulated, premium digital identity zone under the
    supreme jurisdictional authority of the Foundation. To safeguard the integrity, public
    reputation, and absolute security of the alternative root network, the Foundation
    maintains exclusive administrative control over the Master Registry Ledger.

    The Registrant understands that a .rrn domain is not a permanent property right, but a
    conditional, highly regulated operating license. By executing this agreement, the
    Registrant grants the Foundation the absolute, unappealable, and immediate technical
    right to execute the Registry Reclamation Protocol (hereinafter referred to as the "Kill
    Switch") to suspend, revoke, clear, or reclaim any .rrn domain name under the
    conditions set forth in Section 9.02.

    Section 9.02 – Triggers for Immediate Execution of the Kill Switch

    The Foundation, its automated monitoring protocols, or its designated international
    compliance legal teams may trigger the Kill Switch without prior judicial intervention,
    notice, or hearing, upon the occurrence of any of the following Events of Default:

    1. Financial Non-Compliance and Upkeep Delinquency: Failure by the Registrant (or
    its managing Registrar) to settle the tiered enterprise subscription upkeep fees,
    licensing fees, or associated platform balances within exactly zero (0) seconds of the
    specified calendar renewal timestamp.
        
    2. Evasion of Protocol Royalties: Any attempt to bypass, obfuscate, or redirect the
    automated ledger royalty mechanisms during a domain transfer, secondary market
    sale, or commercial sub-domain assignment. All transfers must clear the native
    platform marketplace escrow engine.

    3. Malicious Network Activity and Spoofing: The use of the .rrn domain to host,
    direct, or facilitate phishing campaigns, spoofing, malicious malware distribution,
    decentralized financial (DeFi) scams, or anonymous identity fraud. Because the .rrn
    zone is a contractually guaranteed "Verified Safe Zone," any compromise of public
    trust triggers an instant network response.

    4. Corporate Dissolution or Fraudulent KYC/KYB: The structural bankruptcy, legal
    dissolution, or regulatory stripping of the Registrant’s legal business entity in its
    home jurisdiction, or the discovery of falsified corporate papers, trademark
    documentation, or authorized signatory passport records during Phase 1 onboarding audits.
        
    Section 9.03 – Mechanics of Revocation and Safe Harbor Immunity

    Upon the triggering of an Event of Default, the Foundation shall execute the Kill Switch at
    the core root database level.

    • The cryptographic pointers mapping the .rrn name to the Registrant's servers or
    multi-sig wallets will be severed instantly.

    • Traffic will immediately cease resolving across the alternative non-www root network.

    • The domain name will automatically revert back to the ownership ledger of the
    Foundation for public auction or permanent structural holding.
        """)
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
        root_ip = "IP"
    elif extension == 3:
        name = f"{name}-edu"
        tname = f"{tname}-edu"
        root_server = "edu-ginna-gsl"
        root_ip = "IP"
        id = secrets.token_urlsafe(6)[:6]
        id = f"{id}-EDU-GINA"
    elif extension == 4:
        name = f"{name}-prcn"
        tname = f"{tname}-prcn"
        root_server = "prcn-ginna-gsl"
        root_ip = "IP"
        id = secrets.token_urlsafe(6)[:6]
        id = f"{id}-EDU-GINA"
    elif extension == 5:
        name = f"{name}-edu"
        tname = f"{tname}-edu"
        root_server = "edu-neumann-gsl"
        root_ip = "IP"
        id = secrets.token_urlsafe(6)[:6]
        id = f"{id}-EDU-GINA"
    else:
        print("Invalid Number!")
        return

    file_part = input("Enter folder path: ")

    os.makedirs(file_part, exist_ok=True)
    file_path = f"{tname}.region"
    file_path = f"{file_part}/{file_path}"

    ip = input("Enter IP V4: ").strip()
    ipv6 = input("Enter IP V6: ").strip()

    registrar = input("Enter Registrar Name: ")
    registrar_id = input("Enter Registrar ID: ")

    print("""
1 - 1 year
2 - 2 year
3 - 4 year
4 - 8 year
5 - 10 year
            """)
    expiry = int(input("Enter Expiry: "))

    txt = input("Your Text for your Imperian")

    email = input("Enter Email for Imperian: ")
    if email is None:
        email = ""
    else:
        email = f"{email}@{tname}"

    server1 = "IP"
    server2 = "IP"
    server3 = "IP"

    #rsa - Root Server Address
    #ginna network server
    server_imperian = "thr-exmpl-ext"
    server_imperian2 = "gna-exmpl-ext"
    server_imperian3 = "wirp-exmpl-gns"

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

    private_key = ec.generate_private_key(ec.SECP256K1())
    public_key = private_key.public_key()

    # Serialize public key as BYTES
    public_bytes = public_key.public_bytes(
        encoding=serialization.Encoding.PEM,
        format=serialization.PublicFormat.SubjectPublicKeyInfo
    )

    # Serialize private key as BYTES
    private_bytes = private_key.private_bytes(
        encoding=serialization.Encoding.PEM,
        format=serialization.PrivateFormat.PKCS8,
        encryption_algorithm=serialization.NoEncryption()
    )

    # Hash the BYTES
    public_hash = hashlib.sha256(public_bytes).hexdigest()
    private_hash = hashlib.sha256(private_bytes).hexdigest()

    # Message to sign
    message = f"{name}.{ip}".encode("utf-8")

    # Sign
    signature = private_key.sign(
        message,
        ec.ECDSA(hashes.SHA256())
    ).hex()

    # Registry record
    record = (f"""
Record ID: {id}
Imperian Name: {name}
IP V6: {ipv6}
IP V4: {ip}
Registrar: {registrar}
Registrar ID: {registrar_id}
Public Key: {public_hash}
Signature: {signature}
Created: {now}
Expires: {expiry}
file: {file_path}
"""
    )
#AAOC = Assigned Authority Over Control
#SL = SERVER LOCATION
#INE = IMPERIAN NAME EXPIRES
#INC = IMPERIAN NAME CREATED
#SECKEY = SECRET KEY
#TIN  ==  THE IMPERIAN NAME
#MOIN   =  Mail of Imperian Network Names
#IPT = IP TYPE
#LOC = LOCATION
#RN - Registered Name

    region = f"""$REGION {tname}
@       TIN        {tname}
@       RN         {name}
@       ID         {id}

@       AAOC       Global Internet Names and Numbers Authority
$       IPT        V4 & V6
$       SL         {ip}
$       SLV6       {ipv6}
{var}   LOC        {ip} | {ipv6}
{var}   TXT        {txt}

!       RGSTR      {registrar}
!       RGSTRID    {registrar_id}

Server Location (
    {server_imperian}  A  {server1}
    {server_imperian2}  A  {server2}
    {server_imperian3}  A  {server3}
)

Authoritive Server (
    {root_server} : {root_ip}
)

@      MOINN       {email}
                   
INC     {now}        
INE     {expiry}     
                         
#       SECKEY     {private_bytes}
#       PUBKEY     {public_bytes}
                       
#       SECKEYH    {private_hash}
#       PUBKEYH    {public_hash}
#       SIGN       {signature}

@       TTL        3600
"""
    log = f"""$IMPERIAN LOG {name}
NAME: {tname}
IMPERIAN: {name}
AAOC: Global Internet Names and Numbers Authority
ID: {id}
IP V4: {ip}
IP V6: {ipv6}
REGISTRAR: {registrar}
REGISTRAR ID: {registrar_id}
SL1: {server_imperian} : {server1}
SL2: {server_imperian2} : {server2}
AUTHORITY: {root_server} : {root_ip}
MAIL: {email}
TXT: {txt}
CREATED: {now}
EXPIRY: {expiry}
    """

    file_log = f"{LOG_DIR}/{tname}-name.log"
    # Create data directory if necessary
    os.makedirs(DATA_DIR, exist_ok=True)
    os.makedirs(LOG_DIR, exist_ok=True)

    REQ_PORT = 86
    IP_REQ = ""

    server_req = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        server_req.connect((IP_REQ, REQ_PORT))
        server_req.send(name.encode("utf-8"))
        response_raw, server_address = server_req.recvfrom(4096)
        response_raw = response_raw.decode("utf-8")
        print(response_raw)
        if response_raw == "ISINNE:RNA":
            print("Name Accepted!")
        elif response_raw == "ISINNE:RNE":
            print("Request name already exists!")
            return
        else:
            print("ERROR!")
            return
        
    except ConnectionAbortedError:
        print("Could not connect")
    finally:
        server_req.close()

    # Append record
    with open(CHAIN_FILE, "a", encoding="utf-8") as f:
        f.write(record)

    with open(file_path, "a", encoding="utf-8") as f:
        f.write(region)

    with open(file_log, "a", encoding="utf-8") as f:
        f.write(log)

    print("\nRegistered successfully.")
    print(record)

    root_socket = socket.socket(
        socket.AF_INET,
        socket.SOCK_STREAM
    )
    ROOT_PORT = 70
    try:
        root_socket.connect((root_ip, ROOT_PORT))
        filename = os.path.basename(file_log).encode("utf-8")

        with open(file_log, "rb") as f:
            root_socket.sendall(len(filename).to_bytes(4, "big"))
            root_socket.sendall(filename)

            size = os.path.getsize(file_log)
            root_socket.sendall(size.to_bytes(8, "big"))

            while chunk := f.read(4096):
                root_socket.sendall(chunk)

        print("File sent successfully!")

    except OSError as e:
        print("Transfer failed:", e)

    finally:
        root_socket.close()


    root_socket1 = socket.socket(
        socket.AF_INET,
        socket.SOCK_STREAM
    )
    ROOT_PORT = 80
    try:
        root_socket1.connect((root_ip, ROOT_PORT))
        root_socket1.send(record.encode('utf-8'))
        print("Record is sent to root server")
    except OSError as e:
        print("Transfer failed:", e)

    finally:
        root_socket1.close()

    

    print("SUCCESS!!!")

def registrar():
    name = input("Enter Registrar Name: ")
    tname = name.lower().replace(" ", "")
    tname = re.sub(r'[^a-z0-9]', '', tname)
    file_part = f"{DATA_DIR}/{name}.log"
    file_log = f"{LOG_DIR}/{tname}-rgstr.log"

    file_path = "article.vii"
    if os.path.exists(file_path):
        # Open the file and read its contents safely
        with open(file_path, "r", encoding="utf-8") as f:
            file_content = f.read()

        pydoc.pager(file_content)

    agree = input("Agree with it [y/n]: ")

    if agree == "y":
        print("Continue!")
    elif agree == "n":
        print("You disagree, cannot proceed")
        return
    else:
        return

    country = input("Your Country Code: ")
    number = input("Enter Contact Number: ")
    other_contact = input("Enter other Contact Number: ")
    company_reg = input("Company Name: ")
    abuse_contact = input("Enter Abuse Contact Link: ")
    abuse_phone = input("Enter Abuse Contact Phone Number: ")
    abuse_mail = input("Enter Abuse Mail: ")

    ginnaid = secrets.token_urlsafe(5)[:5]
    print(f"Your GINNA ID: {ginnaid}")

    private_key = ec.generate_private_key(ec.SECP256K1())
    public_key = private_key.public_key()
    
        # Serialize public key as BYTES
    public_bytes = public_key.public_bytes(
        encoding=serialization.Encoding.PEM,
        format=serialization.PublicFormat.SubjectPublicKeyInfo
    )
    
        # Serialize private key as BYTES
    private_bytes = private_key.private_bytes(
        encoding=serialization.Encoding.PEM,
        format=serialization.PrivateFormat.PKCS8,
        encryption_algorithm=serialization.NoEncryption()
    )
    
        # Hash the BYTES
    public_hash = hashlib.sha256(public_bytes).hexdigest()
    private_hash = hashlib.sha256(private_bytes).hexdigest()
    
        # Message to sign
    message = f"{name}".encode("utf-8")
    
        # Sign
    signature = private_key.sign(
        message,
        ec.ECDSA(hashes.SHA256())
    ).hex()

    imperian = f"{tname}-gcr"

    rgstrd = datetime.now()
    expiry = rgstrd + relativedelta(years=1)

    record = (f"""
Name: {name}
Imperian: {imperian}
GINNA ID: {ginnaid}
Public Key: {public_hash}
Signature: {signature}
Registered: {rgstrd}
Expiry: {expiry}
file: {file_part}
""")

#CC = Country Code
    region = f"""$REGISTRAR {name}
@      N        {name}
@      IN       {imperian}
@      AAOC     Global Internet Names and Numbers Authority
@      GID      {ginnaid}

!      COM      {company_reg}
!      CC       {country}
!      NUM      {number}
!      ONUM     {other_contact}

?      ABUSE    {abuse_contact}
?      ABUSEN   {abuse_phone}
?      ABSEE    {abuse_mail}
          
RNC    {rgstrd}
RNE    {expiry}
                   
#      SECKEY   {private_bytes} 
#      PUBKEY   {public_bytes}
       
#      SECKEYH  {private_hash}
#      PUBKEYH  {public_hash}
#      SIGN     {signature}

@VERIFIED BY GINNA, EXPIRES IN {expiry}@
"""

    log = f"""$REGISTRAR LOG {name}
NAME: {name}
IMPERIAN: {imperian}
AAOC: Global Internet Names and Numbers Authority
GINNA ID: {ginnaid}
COMPANY: {company_reg}
COUNTRY: {country}
CONTACT: {number}
OTHER CONTACT: {other_contact}
ABUSE: {abuse_contact}
ABUSE CONTACT: {abuse_phone}
ABUSE EMAIL: {abuse_mail}
CREATION: {rgstrd}
EXPIRY: {expiry}
"""

    os.makedirs(DATA_DIR, exist_ok=True)
    os.makedirs(LOG_DIR, exist_ok=True)
    
        # Append record
    with open(REGISTRAR, "a", encoding="utf-8") as f:
        f.write(record)

    with open(file_part, "a", encoding="utf-8") as f:
        f.write(region)

    with open(file_log, "a", encoding="utf-8") as f:
        f.write(log)

def read_registry():
    if not os.path.exists(CHAIN_FILE):
        print("Registry does not exist.")
        return

    with open(CHAIN_FILE, "r", encoding="utf-8") as f:
        content = f.read()

    records = content.strip().split("\n\n")

    if not records:
        print("Registry is empty.")
        return

    for number, record in enumerate(records, start=1):

        print(f"\n========== Registration {number} ==========")

        data = {}

        for line in record.splitlines():

            if ":" not in line:
                continue

            key, value = line.split(":", 1)

            data[key.strip()] = value.strip()

        print("Record ID:", data.get("Record ID", "N/A"))
        print("Imperian Name:", data.get("Imperian Name", "N/A"))
        print("IP V4:", data.get("IP V4", "N/A"))
        print("IP V6:", data.get("IP V6", "N/A"))
        print("Public Key:", data.get("Public Key", "N/A"))
        print("Registrar:", data.get("Registrar", "N/A"))
        print("Signature:", data.get("Signature", "N/A"))
        print("Created:", data.get("Created", "N/A"))
        print("Expires:", data.get("Expires", "N/A"))

def search_name():
    name = input("Enter Name: ")
    system = input("Enter System Type: ")
    if system == "wddd":
        name = f"idtp/::wddd:{name}"
    elif system == "www":
        name = f"www.{name}"
    else:
        print("Invalid system")
        return
    
    try:
        with open(CHAIN_FILE, "r", encoding="utf-8") as f:
            blocks = f.read().split("\n\n")
    except FileNotFoundError:
        print(f"Error: The file '{CHAIN_FILE}' was not found.")
        return

    for block in blocks:
        block = block.strip()
        if not block:
            continue
            
        record = {}
        for line in block.split("\n"):
            if ":" in line:
                key, val = line.split(":", 1)
                record[key.strip()] = val.strip()
        
        if record.get("Imperian Name") == name:
            ipa = record.get("IP V4", "Not Found")
            ip6 = record.get("IP V6", "Not Found")
            id = record.get("Record ID", "Not Found")
            registrar = record.get("Registrar", "Not Found")
            expires = record.get("Expires", "Not Found")
            created = record.get("Created", "Not Found")
            
            print(f"\nMatch Found for {name}:")
            print(f"Record ID: {id}")
            print(f"IPv4 Address: {ipa}")
            print(f"IPv6 Address: {ip6}")
            print(f"Registrar: {registrar}")
            print(f"Created: {created}")
            print(f"Expires: {expires}")
            return  
            
    print(f"\nNo record found for the name: {name}")

def show_registrar():
    if not os.path.exists(REGISTRAR):
        print("NO REGISTRAR FILE")
        return

    with open(REGISTRAR, "r", encoding="utf-8") as f:
        content = f.read()

    records = content.strip().split("\n\n")

    for number, record in enumerate(records, start=1):
        print(f"\n========== Registrar {number} ==========")

        data = {}
        for line in record.splitlines():
            if ":" in line:
                key, value = line.split(":", 1)
                data[key.strip()] = value.strip()

        print("GINNA ID:", data.get("GINNA ID", "N/A"))
        print("Name:", data.get("Name", "N/A"))
        print("Imperian:", data.get("Imperian", "N/A"))
        print("Signature:", data.get("Signature", "N/A"))
        print("Expiry:", data.get("Expiry", "N/A"))

def start_server():
    global server_process

    if server_process is not None and server_process.poll() is None:
        print("ISINNE server is already running.")
        return

    server_process = subprocess.Popen(
        [sys.executable, "wina.py"],
        stdin=subprocess.DEVNULL,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
        start_new_session=True
    )

    print("ISINNE server running.")


def stop_server():
    global server_process

    if server_process is None or server_process.poll() is not None:
        print("ISINNE server is not running.")
        server_process = None
        return

    server_process.terminate()
    server_process.wait(timeout=5)
    server_process = None

    print("ISINNE server stopped.")

def main():

    while True:

        print("\n")
        print("Global Internet Names and Numbers Authority")
        print("              GINNA AUTHORITY")
        print("register - Sign/Register Imperian name")
        print("registrar - Register Registrar / Create Lease")
        print("read name - Read imperian")
        print("read registrar - Show Registrars")
        print("search name - Search imperian name")
        print("exit - Exit")
        print("isinne [-c, -h, -hc] for more things!")
        print("Internet System for Imperian Names and Numbers Exchange")
        print("(C) Copyright affliated to Rohith & GINNA \n\n")

        command = input("Ginna@ISINNE~$ ").strip()

        if command == "register":
            garnn()

        elif command == "registrar":
            registrar()

        elif command == "read name":
            read_registry()

        elif command == "read registrar":
            show_registrar()

        elif command == "isinned run":
            start_server()

        elif command == "isinned stop":
            stop_server()

        elif command == "search name":
            search_name()

        elif command == "checkregion":
            pass

        elif command == "isinne -c":
            file_path = "commands.txt"
            if os.path.exists(file_path):
                with open(file_path, "r", encoding="utf-8") as f:
                    file_content = f.read()
            
                pydoc.pager(file_content)

        elif command == "exit":
            print("\nGoodbye.")
            break

        else:
            print("Invalid option!")



if __name__ == "__main__":
    main()
