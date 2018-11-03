import React, { Component } from 'react';
import { Button, Modal } from 'react-bootstrap';

import i18n from '../lib/i18n';

class ModalConfirm extends Component {
	constructor(props) {
		super(props);
		
		this.state = {show: props.show, title: props.title, message: props.message, cb: props.onCloseCb};
		
		this.close = this.close.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({show: nextProps.show, title: nextProps.title, message: nextProps.message, cb: nextProps.onCloseCb});
	}

	close(result) {
		this.setState({ show: false }, () => {
			this.state.cb && this.state.cb(result);
		});
	}

	render() {
		return (
			<Modal show={this.state.show}>
				<Modal.Header>
					<Modal.Title>{this.state.title}</Modal.Title>
				</Modal.Header>

				<Modal.Body>
					{this.state.message}
				</Modal.Body>

				<Modal.Footer>
					<Button bsStyle="primary" onClick={() => this.close(true)}>{i18n.t("common.ok")}</Button>
					<Button bsStyle="primary" onClick={() => this.close(false)}>{i18n.t("common.cancel")}</Button>
				</Modal.Footer>

			</Modal>
		);
	}
}

export default ModalConfirm;
