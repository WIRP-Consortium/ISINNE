#Python Trsnslator Prototype
# (C) Rohith. P. B & GINNA

import socket
import threading
import os
import json
import base64
import hashlib
import hmac
import time
import ssl
import ipaddress

from datetime import datetime, timezone

from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import padding
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives.ciphers.aead import AESGCM
from pathlib import Path

def load_config(path):
    config = {}

    if not os.path.exists(path):
        print("[!] Config not found, using defaults")
        return config

    with open(path, "r") as f:
        for line in f:
            line = line.strip()

            if not line:
                continue

            if line.startswith("#"):
                continue

            key, value = line.split(None, 1)

            value = value.strip()

            if value.startswith('"') and value.endswith('"'):
                value = value[1:-1]

            config[key] = value

    return config

CONFIG = load_config("isinned.conf")

HOST = CONFIG.get(
    "BindAddress",
    "0.0.0.0"
)

PORT = int(CONFIG.get(
    "Listen",
    60
))

VER = CONFIG.get(
    "Version",
    "0.01"
)

DATA_DIR = CONFIG.get(
    "DataDirectory",
    "./data"
)

CHAIN_FILE = CONFIG.get(
    "NameFile",
    "name.conf"
)

def answer(ver, name, ip, ip_type, flag, class_type, ttl, type):
    return f"""VERSION: {ver}
NAME: {name}
ADDRESS: {ip}
IP TYPE: {ip_type}
FLAG: {flag}
CLASS: {class_type}
TTL: {ttl}
TYPE: {type}
"""

def parse_message(raw_str):
    data = {}
    for line in raw_str.strip().split("\n"):
        line = line.strip()
        if ":" in line:
            key, val = line.split(":", 1)
            data[key.strip()] = val.strip()
    return data

def request(server_socket, data, client_address):
    print(f"Received request from {client_address}")

    try:
        message_raw = data.decode("utf-8")

        message = parse_message(message_raw)

        name = message.get("NAME")
        protocol_type = message.get("SYSTEM")
        flag = message.get("FLAG")
        ip_type = message.get("IP TYPE")
        class_type = message.get("CLASS")

        if not name or not ip_type or not protocol_type:
            print("Invalid request")
            return

        if protocol_type == "wddd":
            name = f"idtp/::wddd:{name}"
        elif protocol_type == "www":
            name = f"www.{name}"
        else:
            print("Invalid system")
            return

        with open(CHAIN_FILE, "r", encoding="utf-8") as f:
            blocks = f.read().split("\n\n")

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
                if ip_type == "A":
                    ipa = record.get("IP V4")
                elif ip_type == "AAAA":
                    ipa = record.get("IP V6")
                else:
                    ipa = None

                if not ipa:
                    break

                packet = answer(
                    ver=VER,
                    name=name,
                    ip=ipa,
                    ip_type=ip_type,
                    flag=flag,
                    class_type=class_type,
                    ttl="360",
                    type=protocol_type
                )

                server_socket.sendto(
                    packet.encode("utf-8"),
                    client_address
                )
                return

        packet = answer(
            ver=VER,
            name=name,
            ip="NO MATCHING RECORD FOUND",
            ip_type=ip_type,
            flag="NFMPERIAN",
            class_type=class_type,
            ttl="0",
            type=protocol_type
        )

        server_socket.sendto(
            packet.encode("utf-8"),
            client_address
        )

    except Exception as e:
        print(f"Client error from {client_address}: {e}")


def server():

    server_socket = socket.socket(
        socket.AF_INET,
        socket.SOCK_DGRAM
    )
    server_socket.setsockopt(
        socket.SOL_SOCKET,
        socket.SO_REUSEADDR,
        1
    )

    server_socket.bind((HOST, PORT))

    print("Internet System for Imperian Names and Numbers Exchange")
    print(f"UDP listening on {HOST}:{PORT}")

    try:
        while True:
            data, client_address = server_socket.recvfrom(4096)

            client_thread = threading.Thread(
                target=request,
                args=(server_socket, data, client_address),
                daemon=True
            )
            client_thread.start()

    except KeyboardInterrupt:
        print("\nServer stopped!")

    finally:
        server_socket.close()


if __name__ == "__main__":
    server()
