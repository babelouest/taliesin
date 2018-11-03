import React, { Component } from 'react';
import { NavItem } from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';

import StateStore from '../lib/StateStore';
import i18n from '../lib/i18n';

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
		if (this.state.status === "connected" || this.state.status === "connect") {
			return <NavItem href="" onClick={() => this.handleDisconnect()}><FontAwesome name={"sign-out"} /> {i18n.t("topmenu.log_out")}</NavItem>;
		} else if (this.state.status !== "noauth") {
			return <NavItem href="" onClick={() => this.handleConnect()}><FontAwesome name={"sign-in"} /> {i18n.t("topmenu.log_in")}</NavItem>;
		} else {
      return <div></div>;
    }
	}
}

export default LoginButton;
