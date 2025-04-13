import streamlit as st
import pandas as pd
import requests
import datetime
from sklearn.linear_model import LinearRegression
import numpy as np

st.title("SARING - Smart Plant Watering System")

# Get data from Ubidots
UBIDOTS_TOKEN = "BBUS-N7BD5c1zWnvSXahWY92pV7DJniZDtQ"
DEVICE = "saring"

def get_data():
    url = f"https://industrial.api.ubidots.com/api/v1.6/devices/{DEVICE}"
    headers = {"X-Auth-Token": UBIDOTS_TOKEN}
    res = requests.get(url, headers=headers)
    if res.status_code == 200:
        data = res.json()
        kelembapan = data['kelembapan']['last_value']
        pompa = data['pompa']['last_value']
        return kelembapan, pompa
    else:
        return None, None

kelembapan, pompa = get_data()

if kelembapan is not None:
    st.metric("Kelembapan Tanah", f"{kelembapan:.2f}")
    st.metric("Status Pompa", "Aktif" if pompa == 1 else "Mati")

    # Simulasi prediksi AI sederhana (linear regression dummy)
    data_x = np.array([0, 1, 2, 3, 4, 5, 6]).reshape(-1, 1)
    data_y = np.array([650, 600, 550, 500, 450, 400, 350])
    model = LinearRegression()
    model.fit(data_x, data_y)
    pred = model.predict(np.array([[7]]))[0]
    
    st.success(f"Prediksi kelembapan 1 jam lagi: {pred:.2f}")
else:
    st.error("Gagal ambil data dari Ubidots.")
