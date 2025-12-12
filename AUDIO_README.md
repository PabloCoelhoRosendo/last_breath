# 🔊 Sistema de Áudio - Last Breath

## 📁 Estrutura de Arquivos

Coloque seus arquivos de áudio na seguinte estrutura:

```
assets/audio/
├── sfx/                    # Efeitos sonoros
│   ├── tiro_pistol.wav    # Som de tiro da pistola
│   ├── tiro_shotgun.wav   # Som de tiro da shotgun
│   ├── zombie_morte.wav   # Som de morte do zumbi
│   ├── jogador_dano.wav   # Som quando jogador leva dano
│   ├── jogador_morte.wav  # Som de morte do jogador
│   ├── abrir_porta.wav    # Som de abrir porta
│   ├── pegar_moeda.wav    # Som de coletar moeda
│   └── comprar_loja.wav   # Som de comprar na loja
└── music/                  # Músicas (futuro)
    └── (vazio por enquanto)
```

## 🎵 Formato Recomendado

- **Formato**: WAV (sem compressão)
- **Taxa de amostragem**: 44100 Hz
- **Canais**: Mono (1 canal) ou Estéreo (2 canais)
- **Bits**: 16-bit

## ⚙️ Como Funciona

1. **Inicialização**: O sistema de áudio é inicializado no `main.c` com `InitAudioDevice()`
2. **Carregamento**: Os sons são carregados em `recursos.c` usando `LoadSound()`
3. **Uso**: Os sons ficam disponíveis na struct `Recursos` e podem ser tocados com `PlaySound()`
4. **Finalização**: O sistema é fechado com `CloseAudioDevice()`

## 📝 Sons Implementados

| Som | Campo na Struct | Quando Toca |
|-----|----------------|-------------|
| Tiro Pistol | `sfxTiroPistol` | Ao atirar com pistola |
| Tiro Shotgun | `sfxTiroShotgun` | Ao atirar com shotgun |
| Zumbi Morte | `sfxZumbiMorte` | Quando zumbi morre |
| Jogador Dano | `sfxJogadorDano` | Quando jogador leva dano |
| Jogador Morte | `sfxJogadorMorte` | Quando jogador morre |
| Porta | `sfxPorta` | Ao atravessar porta |
| Moeda | `sfxMoeda` | Ao coletar moeda |
| Compra | `sfxCompra` | Ao comprar na loja |

## 🎮 Próximos Passos

Para usar os sons no jogo, você precisará:

1. **Criar ou baixar** os arquivos de áudio WAV
2. **Colocá-los** na pasta `assets/audio/sfx/`
3. **Adicionar chamadas** `PlaySound()` nos momentos apropriados do jogo

Exemplo de uso:
```c
// No momento do tiro
if (recursos->sfxTiroPistol.frameCount > 0) {
    PlaySound(recursos->sfxTiroPistol);
}
```

## ✅ Status

- [x] Sistema de áudio inicializado
- [x] Estrutura de pastas criada
- [x] Sons carregados na struct Recursos
- [ ] Sons conectados aos eventos do jogo
- [ ] Arquivos de áudio adicionados

