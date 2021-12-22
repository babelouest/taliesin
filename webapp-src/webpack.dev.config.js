var path = require('path');

module.exports = {
  entry: './src/index.js',
  devtool: 'inline-source-map',
  output: {
    path: path.resolve(__dirname),
    filename: 'bundle.js',
    libraryTarget: 'umd'
  },

	devServer: {
		contentBase: path.resolve(__dirname),
		compress: true,
		port: 3000,
		host: 'localhost',
		open: true,
    proxy: {
      '/api': {
        target: 'http://localhost:8576/',
        secure: false,
        changeOrigin: true
      },
      '/.well-known/taliesin-configuration': {
        target: 'http://localhost:8576/',
        secure: false,
        changeOrigin: true
      }
    }
	},

  module: {
    loaders: [
      {
        test: /\.js$/,
        exclude: /(node_modules|bower_components|build)/,
        use: {
          loader: 'babel-loader',
          options: {
            presets: ['env']
          }
        }
      },
      {
        test: /\.css$/,
        loader: 'style-loader!css-loader'
      }
    ]
  }
}
