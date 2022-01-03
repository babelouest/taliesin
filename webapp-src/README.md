# Taliesin front-end source files

Taliesin front-end is a single Page Application based on ReactJS/JQuery. Uses Webpack 3 to build, requires npm or yarn.

## Install dependencies

Prior to running the development instance or building the front-end, you must install the dependencies.

```shell
$ npm install
```

## Run development instance

Copy `config.json.sample` to `config.json` and run the webpack dev server:

```shell
$ cp config.json.sample config.json
$ npm run dev
```

Then open the address [http://localhost:3000/](http://localhost:3000/) on your browser.

## Build front-end

```shell
$ make
```

The built web application will be available in `taliesin/webapp`.
