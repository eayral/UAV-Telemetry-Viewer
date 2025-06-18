#!/usr/bin/env python
"""
mavlink_listener.py  –  Mission Planner/SITL TCP akışını dinler,
seçilen telemetri alanlarını Qt arayüzüne satır bazında iletir.

Gönderilen satır örneği:
lat=39.9276543;lon=32.8532101;alt=15.0;gspd=10.03;vspd=0.35;vbatt=12.55
"""

import socket
from pymavlink import mavutil

# ------------------------------------------------------------------
MP_ENDPOINT = 'tcp:127.0.0.1:6000'     # Mission Planner'da açtığın TCP Host
GUI_PORT    = 6000                    # Qt uygulamasının bağlanacağı port

print(f"Connecting MAVLink => {MP_ENDPOINT} ...", flush=True)
mav = mavutil.mavlink_connection(MP_ENDPOINT)

# ------------------ MAVLink stream ayarları -----------------------
print(">> HEARTBEAT bekleniyor", flush=True)
mav.wait_heartbeat()
print(">> HEARTBEAT ALINDI", flush=True)

# Konum paketini 2 Hz gönder diye zorla
mav.mav.command_long_send(
    mav.target_system,
    mav.target_component,
    mavutil.mavlink.MAV_CMD_SET_MESSAGE_INTERVAL,
    0,  # confirmation
    mavutil.mavlink.MAVLINK_MSG_ID_GLOBAL_POSITION_INT,
    2_000_000, 0, 0, 0, 0, 0
)
print(">> MSG_INTERVAL komutu gönderildi (GLOBAL_POSITION_INT @ 2 Hz)", flush=True)

# ------------------ GUI için TCP sunucusu -------------------------
srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
srv.bind(("127.0.0.1", GUI_PORT))
srv.listen(1)
print(f"Waiting GUI on 127.0.0.1:{GUI_PORT}", flush=True)
conn, _ = srv.accept()
print("GUI connected, streaming...", flush=True)

# Son gelen değerleri saklayacağız
latest = {}

# ------------------ Ana döngü -------------------------------------
while True:
    msg = mav.recv_match(blocking=True)
    if not msg:
        continue

    t = msg.get_type()

    if t == 'GLOBAL_POSITION_INT':
        latest['lat'] = msg.lat / 1e7
        latest['lon'] = msg.lon / 1e7
        latest['alt'] = msg.relative_alt / 1000.0        # metre

    elif t == 'VFR_HUD':
        latest['gspd'] = msg.groundspeed                 # m/s
        latest['vspd'] = msg.climb                       # m/s

    elif t == 'SYS_STATUS':
        latest['vbatt'] = msg.voltage_battery / 1000.0   # Volt

    # Gönderilecek zorunlu alanlardan en az biri yoksa atla
    if 'lat' not in latest:
        continue

    # Satırı hazırlayıp gönder
    line = ("lat={lat:.7f};lon={lon:.7f};alt={alt:.1f};"
            "gspd={gspd:.2f};vspd={vspd:.2f};vbatt={vbatt:.2f}").format(
                lat=latest.get('lat', 0.0),
                lon=latest.get('lon', 0.0),
                alt=latest.get('alt', 0.0),
                gspd=latest.get('gspd', 0.0),
                vspd=latest.get('vspd', 0.0),
                vbatt=latest.get('vbatt', 0.0)
            )

    try:
        conn.sendall((line + "\n").encode())
    except (BrokenPipeError, ConnectionResetError):
        # GUI kapandı → tekrar bekle
        print("GUI disconnected, waiting again...", flush=True)
        conn.close()
        conn, _ = srv.accept()
        print("GUI re-connected, streaming...", flush=True)
