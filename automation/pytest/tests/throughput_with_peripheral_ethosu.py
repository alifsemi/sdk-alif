import re
import time

MIN_SPEED = 1.30  # Mbps

def test_throughput_central(DUT1, DUT2):
    """
    DUT1 = peripheral
    DUT2 = central 
    """

   
    while True:
        DUT1.write("tp peripheral")
        DUT1.expect("Start advertising")
        DUT1.write ("ethosu start")
        time.sleep(1)

        DUT2.write("tp central")
        DUT2.expect("Start scanning")
        
        try:
            DUT2.expect("Type 'tp run' to start test", timeout=60)
            print("Central ready to go")
            break  
        except TimeoutError:
            print("Central not ready...")
            time.sleep(2)
            continue  

    DUT2.write("tp run")
    DUT2.expect("Transmit test starts")
    DUT1.expect("Reception starts")

    tx_line = DUT2.expect(r"TRASMIT RESULT:.*Mbps", timeout=1300)
    tx_text = _safe_to_str(tx_line)
    print(f"Transmit result: {tx_text.strip()}")

    rx_line = DUT2.expect(r"RECEIVE RESULT:.*Mbps", timeout=1300)
    rx_text = _safe_to_str(rx_line)
    print(f"Receive result:  {rx_text.strip()}")

    tx_speed = _extract_speed(tx_text)
    rx_speed = _extract_speed(rx_text)

    print("\n=== Throughput Results ===")
    print(f"Transmit speed: {tx_speed:.2f} Mbps")
    print(f"Receive speed:  {rx_speed:.2f} Mbps")

    assert tx_speed >= MIN_SPEED, f"FAIL: Transmit {tx_speed:.2f} Mbps < {MIN_SPEED} Mbps"
    assert rx_speed >= MIN_SPEED, f"FAIL: Receive {rx_speed:.2f} Mbps < {MIN_SPEED} Mbps"
    DUT1.write ("kernel reboot cold")
    DUT2.write ("kernel reboot cold")


def _safe_to_str(value):
    if value is None:
        return ""
    if isinstance(value, bytes):
        return value.decode(errors="ignore")
    if hasattr(value, "group"):  
        return value.group(0)
    return str(value)


def _extract_speed(text):
    if isinstance(text, bytes):
        text = text.decode(errors="ignore")
    match = re.search(r"@ ([\d.]+)\s*Mbps", text)
    if not match:
        print(f"[WARN] No speed Found: {text}")
        return 0.0
    return float(match.group(1))

