import Cookies from 'js-cookie';

class OAuth2Connector {
	
	constructor(parameters) {
		var self = this;
		// internal
		this.refreshToken = false;
		this.accessToken = false;
		this.changeStatusCb = [];
		this.connected = false;
		this.parameters = {};
		if (window.location.pathname !== "/") {
			this.localStorageKey = "taliesinOauth2-" + window.btoa(unescape(encodeURIComponent(window.location.pathname))).replace(/\=+$/m,'');
		} else {
			this.localStorageKey = "taliesinOauth2";
		}
		this.refreshTimeout = false;
		
		if (parameters) {
			this.parameters.storageType = parameters.storageType || "none";
			this.parameters.responseType = parameters.responseType || "code";
			this.parameters.oidcConfig = parameters.oidcConfig || "code";
			this.parameters.scope = parameters.scope || "";
			this.parameters.authUrl = parameters.authUrl || "";
			this.parameters.tokenUrl = parameters.tokenUrl || "";
			this.parameters.clientId = parameters.clientId || "";
			this.parameters.clientPassword = parameters.clientPassword || "";
			this.parameters.redirectUri = parameters.redirectUri || "";
			this.parameters.userinfoUrl = parameters.userinfoUrl || "";
      this.parameters.usePkce = !!parameters.usePkce;
			if (parameters.changeStatusCb) {
				this.changeStatusCb.push(parameters.changeStatusCb);
			}
		}
		
		var storedData;
		var token;
		if (this.parameters.responseType === "token") {
			token = this.getTokenFromFragment();
			if (token) {
				this.accessToken = token;
				document.location = "#";
				this.getConnectedProfile((res, profile) => {
					if (res) {
						this.broadcastMessage("connected", token.access_token, (token.iat + token.expires_in), profile);
					} else {
						this.broadcastMessage("connected", token.access_token, (token.iat + token.expires_in), false);
					}
				});
				this.storeAccessToken(token);
			} else {
				storedData = this.getStoredData();
				if (storedData && storedData.accessToken && this.isTokenValid(storedData.accessToken)) {
					this.accessToken = storedData.accessToken;
					this.broadcastMessage("connected", this.accessToken.access_token, (this.accessToken.iat + this.accessToken.expires_in));
					this.getConnectedProfile((res, profile) => {
						if (res) {
							this.broadcastMessage("profile", profile);
						}
					});
				} else {
					this.broadcastMessage("disconnected");
					this.accessToken = false;
				}
			}
		} else if (this.parameters.responseType === "code") {
			var code = this.getCodeFromQuery();
			if (code) {
				this.getRefreshTokenFromCode(code, function (refreshToken) {
					if (refreshToken) {
						self.refreshToken = refreshToken.refresh_token;
						self.storeRefreshToken(self.refreshToken);
						if (refreshToken.access_token) {
							self.accessToken = {access_token: refreshToken.access_token, iat: refreshToken.iat, expires_in: refreshToken.expires_in};
							self.storeAccessToken(self.accessToken);
							self.broadcastMessage("connected", self.accessToken.access_token, (self.accessToken.iat + self.accessToken.expires_in));
							self.getConnectedProfile((res, profile) => {
								if (res) {
									self.broadcastMessage("profile", profile);
								}
							});
							self.refreshTokenLoop(refreshToken.refresh_token, self.accessToken.expires_in);
						} else {
							self.broadcastMessage("disconnected");
						}
					} else {
						self.broadcastMessage("disconnected");
					}
				});
				window.history.pushState(null, "", document.location.href.split("?")[0]);
			} else {
				storedData = this.getStoredData();
				if (storedData && storedData.accessToken && this.isTokenValid(storedData.accessToken)) {
					this.accessToken = storedData.accessToken;
					this.broadcastMessage("connected", this.accessToken.access_token, (this.accessToken.iat + this.accessToken.expires_in));
					this.getConnectedProfile((res, profile) => {
						if (res) {
							this.broadcastMessage("profile", profile);
						}
					});
					if (storedData.refreshToken) {
						var curDate = new Date();
						var timeout = Math.floor(((self.accessToken.iat + self.accessToken.expires_in)*1000 - curDate.getTime())/1000);
						this.refreshTokenLoop(storedData.refreshToken, timeout);
					}
				} else if (storedData && storedData.refreshToken) {
					this.accessToken = false;
					this.refreshToken = storedData.refreshToken;
					this.executeRefreshToken(storedData.refreshToken, function (result, accessToken) {
						if (result) {
							self.accessToken = accessToken;
							self.storeAccessToken(accessToken);
							self.refreshTokenLoop(storedData.refreshToken, self.accessToken.expires_in);
							self.broadcastMessage("connected", accessToken.access_token, (accessToken.iat + accessToken.expires_in));
							self.getConnectedProfile((res, profile) => {
								if (res) {
									self.broadcastMessage("profile", profile);
								}
							});
						}
					});
				} else {
					this.broadcastMessage("disconnected");
				}
			}
		} else {
			this.broadcastMessage("error");
		}
	}
	
	getQueryParams(qs) {
		qs = qs.split('+').join(' ');

		var params = {},
			tokens,
			re = /[#&]?([^=]+)=([^&]*)/g;

		tokens = re.exec(qs);
		while (tokens) {
			params[decodeURIComponent(tokens[1])] = decodeURIComponent(tokens[2]);
			tokens = re.exec(qs);
		}

		return params;
	}
	
	getTokenFromFragment() {
		var params = this.getQueryParams(document.location.hash);
		var curDate = new Date();
		if (params.access_token && params.expires_in)	{
			return {access_token: params.access_token, expires_in: params.expires_in, iat: Math.floor(curDate.getTime()/1000)};
		} else {
			return false;
		}
	}
	
	getCodeFromQuery() {
		var params = this.getQueryParams(document.location.search.substring(1));
		return params.code||false;
	}
	
	refresh(cb) {
		var self = this;
		if (this.parameters.responseType === "code" && this.refreshToken) {
			this.executeRefreshToken(this.refreshToken, function (result, accessToken) {
				if (result) {
					self.accessToken = accessToken;
					self.storeAccessToken(accessToken);
					self.refreshTokenLoop(self.refreshToken, self.accessToken.expires_in);
					self.broadcastMessage("refresh", accessToken.access_token, (accessToken.iat + accessToken.expires_in));
					cb(accessToken.access_token);
				}
			});
		} else {
			cb(false);
		}
	}
	
	getToken() {
		if (this.accessToken && this.isTokenValid(this.accessToken)) {
			return this.accessToken.access_token;
		} else {
			this.accessToken = false;
			return false;
		}
	}
	
	getStatus() {
		if (this.accessToken && this.isTokenValid(this.accessToken)) {
			return "connected";
		} else {
			this.accessToken = false;
			return "disconnected";
		}
	}
	
	storeAccessToken(token) {
		var storedObject = this.getStoredData();
		if (!storedObject) {
			storedObject = {};
		}
		storedObject.accessToken = token;
		
		if (this.parameters.storageType === "local") {
			return localStorage.setItem(this.localStorageKey, JSON.stringify(storedObject));
		} else if (this.parameters.storageType === "cookie") {
			return Cookies.set(this.localStorageKey, JSON.stringify(storedObject));
		} else {
			return false;
		}
	}
	
	storeRefreshToken(token) {
		var storedObject = this.getStoredData();
		if (!storedObject) {
			storedObject = {};
		}
		storedObject.refreshToken = token;
		
		if (this.parameters.storageType === "local") {
			return localStorage.setItem(this.localStorageKey, JSON.stringify(storedObject));
		} else if (this.parameters.storageType === "cookie") {
			return Cookies.set(this.localStorageKey, JSON.stringify(storedObject));
		} else {
			return false;
		}
	}
	
	getStoredData() {
		var storage;
		if (this.parameters.storageType === "local") {
      try {
        storage = JSON.parse(localStorage.getItem(this.localStorageKey));
        if (storage) {
          return storage;
        } else {
          return {};
        }
      } catch (err) {
        return {};
      }
		} else if (this.parameters.storageType === "cookie") {
      try {
        if (Cookies.get(this.localStorageKey)) {
          return JSON.parse(Cookies.get(this.localStorageKey))||{};
        } else {
          return {};
        }
      } catch (err) {
        return {};
      }
		} else {
			return {};
		}
	}
	
  storePkce(pkce) {
    var storedObject = this.getStoredData();
    if (!storedObject) {
      storedObject = {};
    }
    storedObject.pkce = pkce;

    if (this.parameters.storageType === "local") {
      return localStorage.setItem(this.localStorageKey, JSON.stringify(storedObject));
    } else if (this.parameters.storageType === "cookie") {
      return Cookies.set(this.localStorageKey, JSON.stringify(storedObject));
    } else {
      return false;
    }
  }

  makeRandomString(length) {
    var array            = new Uint8Array(length);
    var result           = '';
    var characters       = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789';
    var charactersLength = characters.length;
    window.crypto.getRandomValues(array);
    for ( var i = 0; i < length; i++ ) {
      while (array[i] >= charactersLength) {
        var oneValue = new Uint8Array(1);
        window.crypto.getRandomValues(oneValue);
        array[i] = oneValue[0];
      }
      result += characters.charAt(array[i]);
   }
   return result;
  }

	onChangeStatus(cb) {
		this.changeStatusCb.push(cb);
	}
	
	isTokenValid(token) {
		if (!!token) {
			var curDate = new Date();
			return ((token.iat + token.expires_in)*1000) > curDate.getTime();
		} else {
			return false;
		}
	}
	
	getRefreshTokenFromCode(code, cb) {
		var self = this;
    var data = {
      grant_type: "authorization_code",
      client_id: this.parameters.clientId,
      redirect_uri: this.parameters.redirectUri,
      code: code};
    var storedData = this.getStoredData();
    if (this.parameters.usePkce && storedData.pkce) {
      data.code_verifier = storedData.pkce;
    }
		$.ajax({
			type: "POST",
			url: this.parameters.tokenUrl,
			data: data,
			success: function (result, status, request) {
				cb(result);
			},
			error: function (error) {
				if (error.status === 403) {
					self.refreshToken = false;
				}
				self.accessToken = false;
				cb(false);
			}
		});
	}
	
	refreshTokenLoop(refreshToken, timeout) {
		var self = this;
		clearTimeout(this.refreshTimeout);
		this.refreshTimeout = setTimeout(function () {
			self.executeRefreshToken(refreshToken, function (res, token) {
				if (res) {
					var curDate = new Date();
					var timeout = Math.floor(((self.accessToken.iat + self.accessToken.expires_in)*1000 - curDate.getTime())/1000);
					self.refreshTokenLoop(refreshToken, timeout);
				}
			});
		}, (timeout - 60)*1000);
	}
	
	runRefreshToken(cb) {
		if (this.getStoredData().refreshToken) {
			return this.executeRefreshToken(this.getStoredData().refreshToken, cb);
		} else {
      return new Promise((resolve, reject) => {
        reject("disconnected");
      });
		}
	}
	
	executeRefreshToken(refreshToken, cb) {
		var self = this;
		return $.ajax({
			type: "POST",
			url: self.parameters.tokenUrl,
			data: {grant_type: "refresh_token", refresh_token: refreshToken},
			success: function (result, status, request) {
				self.accessToken = result;
				self.storeAccessToken(result);
				self.broadcastMessage("refresh", result.access_token, (result.iat + result.expires_in));
				if (cb) {
					cb(true, result);
				}
			},
			error: function (error) {
				if (error.status === 403) {
					self.refreshToken = false;
				}
				self.accessToken = false;
				if (error.readyState === 0) {
					self.broadcastMessage("network error");
				} else {
					self.broadcastMessage("disconnected");
				}
				if (cb) {
					cb(false);
				}
			}
		})
		.then((result) => {
			return {token: result.access_token, expiration: result.expires_in};
		});
	}
	
	broadcastMessage(status, token, expiration, profile) {
		for (var i in this.changeStatusCb) {
			this.changeStatusCb[i](status, token, expiration, profile);
		}
	}
	
	getConnectedProfile(cb) {
		if (this.parameters.userinfoUrl) {
			var self = this;
			$.ajax({
				type: "GET",
				url: self.parameters.userinfoUrl,
				headers: {"Authorization": "Bearer " + self.accessToken.access_token},
				success: function (result) {
					if (cb) {
						cb(true, result)
					}
				},
				error: function (error) {
					if (cb) {
						cb(false)
					}
				}
			});
		} else {
			cb(false);
		}
	}
	
	connect() {
		var token = this.getStoredData();
		if (token && this.isTokenValid(token.accessToken)) {
			this.broadcastMessage("connected", token.accessToken.access_token, (token.accessToken.iat + token.accessToken.expires_in));
			this.getConnectedProfile((res, profile) => {
				if (res) {
					this.broadcastMessage("profile", profile);
				}
			});
		} else {
			token.accessToken = false;
			this.storeAccessToken(false);
			if (this.parameters.responseType === "token") {
				document.location = this.parameters.authUrl + "?response_type=token&client_id=" + this.parameters.clientId + "&redirect_uri=" + this.parameters.redirectUri + "&scope=" + this.parameters.scope;
			} else if (this.parameters.responseType === "code") {
        if (this.parameters.usePkce) {
          let pkce = this.makeRandomString(64);
          this.storePkce(pkce);
          const encoder = new TextEncoder();
          crypto.subtle.digest("SHA-256", encoder.encode(pkce))
          .then(pkceHashed => {
            let pkceHashedB64 = this.base64UrlArrayBuffer(pkceHashed);
            document.location = this.parameters.authUrl + "?response_type=code&client_id=" + this.parameters.clientId + "&redirect_uri=" + this.parameters.redirectUri + "&scope=" + this.parameters.scope + "&code_challenge_method=S256&code_challenge=" + pkceHashedB64;
          });
        } else {
          document.location = this.parameters.authUrl + "?response_type=code&client_id=" + this.parameters.clientId + "&redirect_uri=" + this.parameters.redirectUri + "&scope=" + this.parameters.scope;
        }
			}
		}
	}
	
	disconnect() {
		clearTimeout(this.refreshTimeout);
		if (this.parameters.storageType === "local") {
			localStorage.removeItem(this.localStorageKey);
		} else if (this.parameters.storageType === "cookie") {
			Cookies.remove(this.localStorageKey);
		}
		this.refreshToken = false;
		this.accessToken = false;
		this.broadcastMessage("disconnected");
	}

  /**
   * base64UrlArrayBuffer
   *
   * MIT LICENSE
   *
   * Copyright 2011 Jon Leighton
   * Copyright 2021 Nicolas Mora
   *
   * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
   * to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
   * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
   * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
   * OR OTHER DEALINGS IN THE SOFTWARE.
  */
  base64UrlArrayBuffer(arrayBuffer) {
    var base64    = ''
    var encodings = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_'

    var bytes         = new Uint8Array(arrayBuffer)
    var byteLength    = bytes.byteLength
    var byteRemainder = byteLength % 3
    var mainLength    = byteLength - byteRemainder

    var a, b, c, d
    var chunk

    // Main loop deals with bytes in chunks of 3
    for (var i = 0; i < mainLength; i = i + 3) {
      // Combine the three bytes into a single integer
      chunk = (bytes[i] << 16) | (bytes[i + 1] << 8) | bytes[i + 2]

      // Use bitmasks to extract 6-bit segments from the triplet
      a = (chunk & 16515072) >> 18 // 16515072 = (2^6 - 1) << 18
      b = (chunk & 258048)   >> 12 // 258048   = (2^6 - 1) << 12
      c = (chunk & 4032)     >>  6 // 4032     = (2^6 - 1) << 6
      d = chunk & 63               // 63       = 2^6 - 1

      // Convert the raw binary segments to the appropriate ASCII encoding
      base64 += encodings[a] + encodings[b] + encodings[c] + encodings[d]
    }

    // Deal with the remaining bytes and padding
    if (byteRemainder == 1) {
      chunk = bytes[mainLength]

      a = (chunk & 252) >> 2 // 252 = (2^6 - 1) << 2

      // Set the 4 least significant bits to zero
      b = (chunk & 3)   << 4 // 3   = 2^2 - 1

      base64 += encodings[a] + encodings[b]
    } else if (byteRemainder == 2) {
      chunk = (bytes[mainLength] << 8) | bytes[mainLength + 1]

      a = (chunk & 64512) >> 10 // 64512 = (2^6 - 1) << 10
      b = (chunk & 1008)  >>  4 // 1008  = (2^6 - 1) << 4

      // Set the 2 least significant bits to zero
      c = (chunk & 15)    <<  2 // 15    = 2^4 - 1

      base64 += encodings[a] + encodings[b] + encodings[c]
    }

    return base64
  }
}

export default OAuth2Connector;
