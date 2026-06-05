import tkinter as tk
from tkinter import ttk
import psutil

# -------------------------
# Fenêtre principale
# -------------------------
root = tk.Tk()
root.title("Monitoring système")
root.geometry("420x280")
root.resizable(False, False)

# -------------------------
# Titre principal
# -------------------------
title = tk.Label(
    root,
    text="Surveillance des ressources système",
    font=("Arial", 14, "bold")
)
title.pack(pady=10)

# -------------------------
# Fonction pour créer une ligne (label + barre + %)
# -------------------------
def create_bar(parent, text):
    frame = tk.Frame(parent)
    frame.pack(fill="x", padx=20, pady=5)

    label = tk.Label(frame, text=text, width=6)
    label.pack(side="left")

    bar = ttk.Progressbar(
        frame,
        orient="horizontal",
        length=220,
        mode="determinate",
        maximum=100
    )
    bar.pack(side="left", padx=10)

    percent = tk.Label(frame, text="0%", width=5)
    percent.pack(side="right")

    return bar, percent

# -------------------------
# Création des barres
# -------------------------
bar_ram,  txt_ram  = create_bar(root, "RAM")
bar_cpu,  txt_cpu  = create_bar(root, "CPU")
bar_disk, txt_disk = create_bar(root, "DISK")

# -------------------------
# Zone d'alertes
# -------------------------
alert_frame = tk.Frame(root, bg="#fff3cd", bd=1, relief="solid")
alert_label = tk.Label(
    alert_frame,
    text="",
    bg="#fff3cd",
    fg="#856404",
    font=("Arial", 10),
    justify="left",
    wraplength=370,
    padx=8,
    pady=6
)
alert_label.pack()

SEUIL = 80

# -------------------------
# Mise à jour INFINIE
# -------------------------
def update():
    ram  = psutil.virtual_memory().percent
    cpu  = psutil.cpu_percent()
    disk = psutil.disk_usage('/').percent

    # --- Mise à jour des barres ---
    for bar, txt, val, nom in [
        (bar_ram,  txt_ram,  ram,  "RAM"),
        (bar_cpu,  txt_cpu,  cpu,  "CPU"),
        (bar_disk, txt_disk, disk, "DISK"),
    ]:
        bar["value"] = val
        txt.config(text=f"{val}%")

        # Couleur de la barre selon le seuil
        style_name = f"{nom}.Horizontal.TProgressbar"
        color = "#dc3545" if val > SEUIL else "#28a745"
        style.configure(style_name, troughcolor="#e9ecef", background=color)
        bar.configure(style=style_name)

    # --- Construction du message d'alerte ---
    alertes = []
    if ram  > SEUIL: alertes.append(f"⚠ RAM  : {ram}% (> {SEUIL}%)")
    if cpu  > SEUIL: alertes.append(f"⚠ CPU  : {cpu}% (> {SEUIL}%)")
    if disk > SEUIL: alertes.append(f"⚠ DISK : {disk}% (> {SEUIL}%)")

    if alertes:
        alert_label.config(text="\n".join(alertes))
        alert_frame.pack(fill="x", padx=20, pady=(4, 8))
    else:
        alert_frame.pack_forget()

    root.after(800, update)

# -------------------------
# Styles de barres colorées
# -------------------------
style = ttk.Style()
for nom in ["RAM", "CPU", "DISK"]:
    style.configure(
        f"{nom}.Horizontal.TProgressbar",
        troughcolor="#e9ecef",
        background="#28a745"
    )

# Lancement de la boucle
update()

# -------------------------
# Lancement TK
# -------------------------
root.mainloop()
