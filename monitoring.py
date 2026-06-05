import tkinter as tk
from tkinter import ttk
import psutil

# -------------------------
# Fenêtre principale
# -------------------------
root = tk.Tk()
root.title("Monitoring système")
root.geometry("420x220")
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
# Mise à jour INFINIE
# -------------------------
def update():
    ram  = psutil.virtual_memory().percent
    cpu  = psutil.cpu_percent()
    disk = psutil.disk_usage('/').percent

    bar_ram["value"]  = ram
    bar_cpu["value"]  = cpu
    bar_disk["value"] = disk

    txt_ram.config(text=f"{ram}%")
    txt_cpu.config(text=f"{cpu}%")
    txt_disk.config(text=f"{disk}%")

    # rappelle update() toutes les 800 ms
    root.after(800, update)

# Lancement de la boucle
update()

# -------------------------
# Lancement TK
# -------------------------
root.mainloop()