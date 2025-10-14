from .sub20_defs import *
import os


# ---------- logger a project/reg/log/sub20.log ----------
def _log_hw(msg: str) -> None:
    # .../Drivers/sub20_py/sub20_port.py -> .../project
    proj_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
    log_dir  = os.path.join(proj_dir, "reg", "log")
    os.makedirs(log_dir, exist_ok=True)
    with open(os.path.join(log_dir, "sub20.log"), "a", encoding="utf-8") as f:
        f.write(msg.rstrip() + "\n")


class Sub20:
    """
    Driver SUB-20 orientado a uso continuo:
    - Abre una vez (constructor), configura una vez.
    - Operaciones NO reabren ni reconfiguran.
    - Con `with` se cierra solo.
    """

    # ---------- construcción y contexto ----------
    def __init__(self, sn: str | None = None):
        self.h = None  # SIEMPRE definida
        self.sn = sn

        dev = sub_find_devices(None)
        while dev:
            h = sub_open(dev)
            if not h:
                _log_hw(f"Open Fail (errno={getattr(sub_errno, 'value', 'NA')})")
            else:
                try:
                    if self._compare_sn(h, sn):
                        self.h = h
                        _log_hw(f"SUB-20 abierto (sn={_sn_from_handle(h)})")
                        break
                finally:
                    if self.h is None:
                        sub_close(h)
            dev = sub_find_devices(dev)

        if self.h is None:
            _log_hw(f"No SUB-20 found (errno={getattr(sub_errno, 'value', 'NA')})")

    def __enter__(self):
        return self

    def __exit__(self, et, ev, tb):
        self.close()

    def close(self):
        if getattr(self, "h", None):
            sub_close(self.h)
            self.h = None
            _log_hw("SUB-20: handle cerrada")

    # ---------- helpers internos ----------
    def _require(self) -> bool:
        if not getattr(self, "h", None):
            _log_hw("Handle cerrado o inexistente: operación omitida")
            return False
        return True

    @staticmethod
    def _bytes_from(buf, n):
        return bytes((c_ubyte * n).from_buffer_copy(buf))

    # ---------- SN / enumeración ---------
    def get_sn(self) -> str:
        if not self._require():
            return ""
        return _sn_from_handle(self.h)

    @staticmethod
    def _compare_sn(h, sn: str | None) -> bool:
        if sn is None:
            return True
        if not h:
            return False
        b = create_string_buffer(32)
        rc = sub_get_serial_number(h, b, len(b))
        if rc < 0:
            return False
        return b.value.decode(errors="ignore").strip() == sn

    # ---------- I²C: config + operaciones ----------
    def i2c_set_freq(self, freq: int = 100_000) -> int:
        if not self._require():
            return -1
        rc = sub_i2c_freq(self.h, byref(c_int(int(freq))))
        if rc < 0:
            _log_hw(f"I2C freq error (errno={sub_errno.value})")
        else:
            _log_hw(f"I2C freq set to {freq} Hz")
        return rc

    def scan_i2c(self) -> list[str]:
        if not self._require():
            return []
        n = c_int(0)
        buf = create_string_buffer(256)
        rc = sub_i2c_scan(self.h, byref(n), buf)
        if rc < 0 or n.value <= 0:
            _log_hw("I2C scan: vacío o error")
            return []
        raw = list(buf.raw[:n.value])
        addrs7 = sorted({ (b >> 1) & 0x7F for b in raw if b not in (0x00, 0xFF) })
        _log_hw(f"I2C scan: {', '.join(hex(a) for a in addrs7)}")
        return [hex(a) for a in addrs7]

    @staticmethod
    def _coerce_bytes(data) -> bytes:
        if data is None:
            return b""
        if isinstance(data, bytes):
            return data
        if isinstance(data, bytearray):
            return bytes(data)
        if isinstance(data, memoryview):
            return data.tobytes()
        if isinstance(data, int):
            if data < 0 or data > 0xFF:
                raise ValueError("int fuera de 0..255")
            return bytes([data])
        if isinstance(data, (list, tuple)):
            return bytes(int(x) & 0xFF for x in data)
        if isinstance(data, str):
            parts = data.replace(",", " ").split()
            return bytes(int(p, 0) & 0xFF for p in parts)
        raise TypeError(f"Tipo no soportado para data: {type(data)}")

    @staticmethod
    def _auto_ma_sz(ma: int | None, ma_sz: int | None) -> tuple[int, int]:
        if ma is None:
            return 0, 0
        ma_int = int(ma)
        if ma_sz not in (1, 2, 3, 4):
            if   ma_int <= 0xFF:       ma_sz = 1
            elif ma_int <= 0xFFFF:     ma_sz = 2
            elif ma_int <= 0xFFFFFF:   ma_sz = 3
            else:                      ma_sz = 4
        return ma_int, ma_sz

    def i2c_writell(self, sa: int, data, ma: int | None = None, ma_sz: int = 0) -> int:
        if not self._require():
            return -1
        payload = self._coerce_bytes(data)
        _ma, _masz = self._auto_ma_sz(ma, ma_sz)
        buf = (c_char * len(payload)).from_buffer_copy(payload)
        rc = sub_i2c_write(self.h, int(sa), _ma, _masz, buf, len(payload))
        if rc < 0:
            _log_hw(f"I2C write ERROR sa=0x{int(sa)&0x7F:02X} ma=0x{_ma:X} ma_sz={_masz} len={len(payload)} (errno={sub_errno.value})")
        else:
            _log_hw(f"I2C write OK    sa=0x{int(sa)&0x7F:02X} ma=0x{_ma:X} ma_sz={_masz} len={len(payload)}")
        return rc

    def i2c_readll(self, sa: int, size: int, ma: int | None = None, ma_sz: int = 0) -> bytes:
        if not self._require():
            return b""
        _ma, _masz = self._auto_ma_sz(ma, ma_sz)
        out = (c_char * int(size))()
        rc = sub_i2c_read(self.h, int(sa), _ma, _masz, out, int(size))
        if rc < 0:
            _log_hw(f"I2C read  ERROR sa=0x{int(sa)&0x7F:02X} ma=0x{_ma:X} ma_sz={_masz} len={int(size)} (errno={sub_errno.value})")
            return b""
        _log_hw(f"I2C read  OK    sa=0x{int(sa)&0x7F:02X} ma=0x{_ma:X} ma_sz={_masz} len={int(size)}")
        return bytes(out[:int(size)])

    def i2c_transfer(self, sa: int, wr: bytes, rd_size: int) -> bytes:
        if not self._require():
            return b""
        wbytes = self._coerce_bytes(wr)
        wbuf = (c_char * len(wbytes)).from_buffer_copy(wbytes) if wbytes else None
        rbuf = (c_char * int(rd_size))()
        rc = sub_i2c_transfer(self.h, int(sa), wbuf, len(wbytes), rbuf, int(rd_size))
        if rc < 0:
            _log_hw(f"I2C xfer  ERROR sa=0x{int(sa)&0x7F:02X} wrlen={len(wbytes)} rdlen={int(rd_size)} (errno={sub_errno.value})")
            return b""
        _log_hw(f"I2C xfer  OK    sa=0x{int(sa)&0x7F:02X} wrlen={len(wbytes)} rdlen={int(rd_size)}")
        return bytes(rbuf[:int(rd_size)])

    # ---------- SPI ----------
    def spi_set_config(self, flags: int) -> int:
        if not self._require():
            return -1
        rc = sub_spi_config(self.h, int(flags), None)
        if rc < 0:
            _log_hw(f"SPI config error (errno={sub_errno.value})")
        else:
            _log_hw(f"SPI config OK flags=0x{int(flags):X}")
        return rc

    def spi_rw(self, out: bytes, ess: str = "0L") -> bytes:
        if not self._require():
            return b""
        outbuf = (c_char * len(out)).from_buffer_copy(out)
        inbuf  = (c_char * len(out))()
        rc = sub_spi_transfer_ess(self.h, outbuf, inbuf, len(out), ess.encode())
        if rc < 0:
            _log_hw(f"SPI RW error (errno={sub_errno.value})")
            return b""
        _log_hw(f"SPI RW OK len={len(out)} ess={ess}")
        return bytes(inbuf[:len(out)])

    def spi_write(self, out: bytes, ess: str = "0L") -> int:
        if not self._require():
            return -1
        outbuf = (c_char * len(out)).from_buffer_copy(out)
        rc = sub_spi_transfer_ess(self.h, outbuf, None, len(out), ess.encode())
        if rc < 0:
            _log_hw(f"SPI W  error (errno={sub_errno.value})")
        else:
            _log_hw(f"SPI W  OK len={len(out)} ess={ess}")
        return rc

    def spi_read(self, nbytes: int, ess: str = "0L") -> bytes:
        if not self._require():
            return b""
        inbuf = (c_char * int(nbytes))()
        rc = sub_spi_transfer_ess(self.h, None, inbuf, int(nbytes), ess.encode())
        if rc < 0:
            _log_hw(f"SPI R  error (errno={sub_errno.value})")
            return b""
        _log_hw(f"SPI R  OK len={int(nbytes)} ess={ess}")
        return bytes(inbuf[:int(nbytes)])

    # ---------- GPIO ----------
    def gpio_set_config(self, set_bits: int, mask: int, bank: int = 0) -> int:
        if not self._require():
            return -1
        s, g, m = c_int(set_bits), c_int(0), c_int(mask)
        rc = sub_gpio_config(self.h, s, byref(g), m) if bank == 0 \
             else sub_gpiob_config(self.h, s, byref(g), m)
        if rc < 0:
            _log_hw(f"GPIO cfg error (errno={sub_errno.value})")
        else:
            _log_hw(f"GPIO cfg OK set=0x{set_bits:X} mask=0x{mask:X} bank={bank}")
        return rc

    def gpio_read(self, bank: int = 0) -> int:
        if not self._require():
            return 0
        g = c_int(0)
        rc = sub_gpio_read(self.h, byref(g)) if bank == 0 \
             else sub_gpiob_read(self.h, byref(g))
        if rc < 0:
            _log_hw(f"GPIO read error (errno={sub_errno.value})")
            return 0
        _log_hw(f"GPIO read OK val=0x{g.value & 0xFFFFFFFF:08X} bank={bank}")
        return g.value & 0xFFFFFFFF

    def gpio_write(self, set_bits: int, mask: int, bank: int = 0) -> int:
        if not self._require():
            return -1
        s, g, m = c_int(set_bits), c_int(0), c_int(mask)
        rc = sub_gpio_write(self.h, s, byref(g), m) if bank == 0 \
             else sub_gpiob_write(self.h, s, byref(g), m)
        if rc < 0:
            _log_hw(f"GPIO write error (errno={sub_errno.value})")
        else:
            _log_hw(f"GPIO write OK set=0x{set_bits:X} mask=0x{mask:X} bank={bank}")
        return rc

    # ---------- atajos de registro ----------
    def i2c_write(self, sa: int, reg: int, data) -> int:
        return self.i2c_writell(sa, data, ma=reg, ma_sz=1)

    def i2c_read(self, sa: int, reg: int, size: int) -> bytes:
        return self.i2c_readll(sa, size, ma=reg, ma_sz=1)

    def i2c_write_reg16(self, sa: int, reg: int, data) -> int:
        return self.i2c_writell(sa, data, ma=reg, ma_sz=2)

    def i2c_read_reg16(self, sa: int, reg: int, size: int) -> bytes:
        return self.i2c_readll(sa, size, ma=reg, ma_sz=2)

@staticmethod
def _sn_from_handle(h) -> str:
    b = create_string_buffer(32)
    rc = sub_get_serial_number(h, b, len(b))
    return b.value.decode(errors="ignore").strip() if rc >= 0 else ""

def sub20_list_sns() -> list[str]:
    """
    Enumerate all connected SUB-20 serial numbers.

    Example:
        >>> sub20_list_sns()
        ['8363', 'ABCD', ...]
    """
    sns = []
    dev = sub_find_devices(None)
    while dev:
        h = sub_open(dev)
        if h:
            try:
                sn = _sn_from_handle(h)
                if sn:
                    sns.append(sn)
            finally:
                sub_close(h)
        dev = sub_find_devices(dev)
    return sns
