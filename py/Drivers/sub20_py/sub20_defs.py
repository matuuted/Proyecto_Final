from ctypes import *
import sys, os, importlib, importlib.util

if sys.platform == "win32":
    os.add_dll_directory(r"C:\Windows\System32")

def load_sub20():
    """Load pysub20 if available, otherwise load the DLL directly."""
    if importlib.util.find_spec("sub20"):
        from sub20.ctypeslib.libsub import SIGNATURES
        from sub20.ctypeslib.utils import load_ctypes_library
        libname = "sub20.dll" if sys.platform == "win32" else "libsub.so"
        li2c = load_ctypes_library(libname, SIGNATURES)
    else:
        libname = "sub20.dll" if sys.platform == "win32" else "libsub.so"
        li2c = WinDLL(libname) if sys.platform == "win32" else cdll.LoadLibrary(libname)
    return li2c, c_int.in_dll(li2c, "sub_errno")

li2c, sub_errno = load_sub20()


# ================================
#   FUNCIONES SUB-20 - DEFINICIONES
# ================================

# ----------- Mensajes de error ----------
sub_strerror = li2c.sub_strerror
sub_strerror.argtypes = [c_int]
sub_strerror.restype = c_char_p


# ----------- Find / Open / Close ----------
sub_find_devices = li2c.sub_find_devices
sub_find_devices.argtypes = [c_void_p]
sub_find_devices.restype = c_void_p

sub_open = li2c.sub_open
sub_open.argtypes = [c_void_p]
sub_open.restype = c_void_p

sub_close = li2c.sub_close
sub_close.argtypes = [c_void_p]
sub_close.restype = None


# ----------- Serial Number ----------
sub_get_serial_number = li2c.sub_get_serial_number
sub_get_serial_number.argtypes = [
    c_void_p,        # handle
    POINTER(c_char), # buffer
    c_int            # buffer length
]
sub_get_serial_number.restype = c_int


# ================================
#   I²C
# ================================
sub_i2c_freq = li2c.sub_i2c_freq
sub_i2c_freq.argtypes = [
    c_void_p,        # handle
    POINTER(c_int)   # frecuencia
]
sub_i2c_freq.restype = c_int

sub_i2c_scan = li2c.sub_i2c_scan
sub_i2c_scan.argtypes = [
    c_void_p,        # handle
    POINTER(c_int),  # cantidad encontrada
    POINTER(c_char)  # buffer salida
]
sub_i2c_scan.restype = c_int

sub_i2c_read = li2c.sub_i2c_read
sub_i2c_read.argtypes = [
    c_void_p,        # handle
    c_int,           # slave address (7-bit)
    c_int,           # memory address opcional
    c_int,           # tamaño de memory address
    POINTER(c_char), # buffer destino
    c_int            # cantidad de bytes a leer
]
sub_i2c_read.restype = c_int

sub_i2c_write = li2c.sub_i2c_write
sub_i2c_write.argtypes = [
    c_void_p,        # handle
    c_int,           # slave address (7-bit)
    c_int,           # memory address opcional
    c_int,           # tamaño de memory address
    POINTER(c_char), # buffer origen
    c_int            # cantidad de bytes a escribir
]
sub_i2c_write.restype = c_int

sub_i2c_transfer = li2c.sub_i2c_transfer
sub_i2c_transfer.argtypes = [
    c_void_p,        # handle
    c_int,           # slave address
    POINTER(c_char), # buffer write
    c_int,           # tamaño write
    POINTER(c_char), # buffer read
    c_int            # tamaño read
]
sub_i2c_transfer.restype = c_int


# ================================
#   SPI
# ================================
sub_spi_config = li2c.sub_spi_config
sub_spi_config.argtypes = [
    c_void_p,        # handle
    c_int,           # flags config
    POINTER(c_int)   # flags actuales (puede ser NULL)
]
sub_spi_config.restype = c_int

sub_spi_transfer_ess = li2c.sub_spi_transfer_ess
sub_spi_transfer_ess.argtypes = [
    c_void_p,        # handle
    POINTER(c_char), # out buffer
    POINTER(c_char), # in buffer
    c_int,           # cantidad de bytes
    c_char_p         # ESS string
]
sub_spi_transfer_ess.restype = c_int


# ================================
#   GPIO
# ================================
sub_gpio_config = li2c.sub_gpio_config
sub_gpio_config.argtypes = [
    c_void_p,        # handle
    c_int,           # set
    POINTER(c_int),  # g (placeholder)
    c_int            # mask
]
sub_gpio_config.restype = c_int

sub_gpio_read = li2c.sub_gpio_read
sub_gpio_read.argtypes = [
    c_void_p,        # handle
    POINTER(c_int)   # value
]
sub_gpio_read.restype = c_int

sub_gpio_write = li2c.sub_gpio_write
sub_gpio_write.argtypes = [
    c_void_p,        # handle
    c_int,           # set
    POINTER(c_int),  # g (placeholder)
    c_int            # mask
]
sub_gpio_write.restype = c_int


# ----------- GPIO Bank B -----------
sub_gpiob_config = li2c.sub_gpiob_config
sub_gpiob_config.argtypes = [
    c_void_p,        # handle
    c_int,           # set
    POINTER(c_int),  # g (placeholder)
    c_int            # mask
]
sub_gpiob_config.restype = c_int

sub_gpiob_read = li2c.sub_gpiob_read
sub_gpiob_read.argtypes = [
    c_void_p,        # handle
    POINTER(c_int)   # value
]
sub_gpiob_read.restype = c_int

sub_gpiob_write = li2c.sub_gpiob_write
sub_gpiob_write.argtypes = [
    c_void_p,        # handle
    c_int,           # set
    POINTER(c_int),  # g (placeholder)
    c_int            # mask
]
sub_gpiob_write.restype = c_int