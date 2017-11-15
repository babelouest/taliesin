import React, { Component } from 'react';
import { NavItem } from 'react-bootstrap';
import { Glyphicon } from 'react-bootstrap';
import StateStore from '../lib/StateStore';

class LoginButton extends Component {
  constructor(props) {
    super(props);
		this.state = {status: StateStore.getState().status};
		
    this.handleDisconnect = this.handleDisconnect.bind(this);
    this.handleConnect = this.handleConnect.bind(this);
		StateStore.subscribe(() => {
			if (StateStore.getState().lastAction === "connection") {
				this.setState({status: StateStore.getState().status});
			}
		});
  }
  
  handleDisconnect(event) {
		StateStore.getState().oauth2Connector.disconnect();
  }

  handleConnect(event) {
    StateStore.getState().oauth2Connector.connect();
  }
  
  render () {
		if (this.state.status === "connected") {
			return <NavItem href="" onClick={() => this.handleDisconnect()}><Glyphicon glyph="log-out"/> Log out</NavItem>;
		} else {
			return <NavItem href="" onClick={() => this.handleConnect()}><Glyphicon glyph="log-in"/> Log in</NavItem>;
		}
  }
}

export default LoginButton;
