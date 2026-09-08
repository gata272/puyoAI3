# PuyoAI3

ぷよぷよシミュレータに統合するための新しいAI実装です。

## 現在のAI

### 1. GTR構築

既存 `puyoAI2` のGTR構築ロジックを分離して保持しています。

- 3手分のツモから色を抽象化
- GTR用パターンを判定
- パターンに対応する3手の固定プランを生成

### 2. GTR後

GTR構築後は固定手順ではなく、

- 3手先のBeam Search
- amaの公開 `build` プロファイルの線形評価重み
- quiescence-style tactical evaluation
- 盤面シミュレーション

を使用します。

評価特徴量は `shape / well / bump / form / link_2 / link_3 / waste_14 / side / nuisance` と、
先読みで評価する `chain / y / key / chi`、操作由来の `tear / waste` です。

> 注意: これは ama のビットフィールド・人間形パターン・quiet search を完全にコピーしたものではありません。
> 公開された評価式と重みを基礎に、WebAssembly上で動かしやすい独立実装にしています。

## Web / オンライン機能

既存の `puyoSim.js`、`online.js`、`online.css` を残しているため、PeerJSを利用したオンライン対戦機能は維持します。

ブラウザ上では、

`puyoSim.js → puyoAI.js → Worker → WASM → AI`

という経路でAIを実行します。

## ビルド

GitHub ActionsでEmscriptenを取得し、WASMを生成してGitHub Pagesへデプロイします。

ローカルでAIコアだけ確認する場合:

```bash
make test
```

## ディレクトリ

```text
ai/
  ai.cpp
  gtr/
  evaluation/
  search/
  simulation/

wasm/
  puyoAI.cpp

puyoSim.js
online.js
online.css
puyoAI.js
puyo-ai-worker-wasm.js

config/
  weights.json

docs/
  ARCHITECTURE.md
  AMA_EVALUATION.md
  MIGRATION.md
```
