import streamlit as st
import pandas as pd
import requests
import datetime
from sklearn.linear_model import LinearRegression
import numpy as np

st.title("SARING - Smart Plant Watering System")

# Get data from Ubidots
UBIDOTS_TOKEN = "BBUS-N7BD5c1zWnvSXahWY92pV7DJniZDtQ"
device_label = "saring"

def get_data():
    headers = {"X-Auth-Token": "BBUS-N7BD5c1zWnvSXahWY92pV7DJniZDtQ"}
    
    url_kelembapan = f"https://industrial.api.ubidots.com/api/v1.6/devices/{device_label}/kelembapan/lv"
    url_pompa = f"https://industrial.api.ubidots.com/api/v1.6/devices/{device_label}/pompa/lv"
    
    res_kelembapan = requests.get(url_kelembapan, headers=headers)
    res_pompa = requests.get(url_pompa, headers=headers)

    kelembapan = res_kelembapan.text if res_kelembapan.status_code == 200 else None
    pompa = res_pompa.text if res_pompa.status_code == 200 else None
    
    return kelembapan, pompa

kelembapan, pompa = get_data()

# Tampilkan data jika berhasil didapat
if kelembapan is not None:
    try:
        kelembapan = float(kelembapan)
        st.metric("Kelembapan Tanah", f"{kelembapan:.2f}")
    except ValueError:
        st.metric("Kelembapan Tanah", "Data tidak valid")

    # Cek status pompa
    if pompa is not None:
        try:
            pompa = int(pompa)
            st.metric("Status Pompa", "Aktif" if pompa == 1 else "Mati")
        except ValueError:
            st.metric("Status Pompa", "Data tidak valid")
    else:
        st.metric("Status Pompa", "Tidak tersedia")

    # Simulasi prediksi AI sederhana
    data_x = np.array([0, 1, 2, 3, 4, 5, 6]).reshape(-1, 1)
    data_y = np.array([650, 600, 550, 500, 450, 400, 350])
    model = LinearRegression()
    model.fit(data_x, data_y)
    pred = model.predict(np.array([[7]]))[0]
    
    st.success(f"Prediksi kelembapan 1 jam lagi: {pred:.2f}")
else:
    st.error("Gagal ambil data dari Ubidots.")
