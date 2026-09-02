import pandas as pd
import matplotlib.pyplot as plt

# Leer los datos exportados por C++
# Usamos skipinitialspace=True por si hay espacios después de las comas en tu CSV
df = pd.read_csv('simulacion.csv', skipinitialspace=True)

# Crear una figura con dos subgráficos
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 10))

# Gráfico 1: Evolución de la Población
ax1.plot(df['Generacion'], df['Poblacion'], color='black', marker='o')
ax1.set_title('Evolución del Tamaño de la Población')
ax1.set_xlabel('Generación')
ax1.set_ylabel('Número de Individuos')
ax1.grid(True, linestyle='--', alpha=0.7)

# Gráfico 2: Evolución de las Atracciones Medias (4 variables)
ax2.plot(df['Generacion'], df['AtraccionMediaHombreAMujer'] * 100, 
         label='Hombres hacia Mujeres', color='blue', linestyle='-')
ax2.plot(df['Generacion'], df['AtraccionMediaHombreAHombre'] * 100, 
         label='Hombres hacia Hombres', color='blue', linestyle='--')

ax2.plot(df['Generacion'], df['AtraccionMediaMujerAHombre'] * 100, 
         label='Mujeres hacia Hombres', color='red', linestyle='-')
ax2.plot(df['Generacion'], df['AtraccionMediaMujerAMujer'] * 100, 
         label='Mujeres hacia Mujeres', color='red', linestyle='--')

ax2.set_title('Evolución de los Rasgos de Atracción Cruzados')
ax2.set_xlabel('Generación')
ax2.set_ylabel('Atracción Media (%)')
ax2.legend()
ax2.grid(True, linestyle='--', alpha=0.7)

plt.tight_layout()
plt.show()