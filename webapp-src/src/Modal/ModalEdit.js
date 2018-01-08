import React, { Component } from 'react';
import { Button, Modal, FormControl, FormGroup, ControlLabel } from 'react-bootstrap';
import i18n from '../lib/i18n';

class ModalEdit extends Component {
	constructor(props) {
		super(props);
		
		this.state = {show: props.show, title: props.title, message: props.message, value: props.value, cb: props.onCloseCb};
		
		this.close = this.close.bind(this);
		this.handleChange = this.handleChange.bind(this);
		this.getValidationState = this.getValidationState.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({show: nextProps.show, title: nextProps.title, message: nextProps.message, value: nextProps.value, cb: nextProps.onCloseCb});
	}

	close(result, e) {
		if (e) {
			e.preventDefault();
		}
		this.setState({ show: false }, () => {
			this.state.cb && this.state.cb(result?this.state.value:false);
		});
	}
	
	getValidationState() {
		if (!this.state.value || !this.state.value.length) {
			return "error";
		} else {
			return "success";
		}
	}

	handleChange(e) {
		this.setState({ value: e.target.value });
	}

	render() {
		return (
			<Modal show={this.state.show}>
				<Modal.Header>
					<Modal.Title>{this.state.title}</Modal.Title>
				</Modal.Header>

				<Modal.Body>
					<form onSubmit={(e) => {this.close(true, e)}}>
						<FormGroup
							controlId="formBasicText"
							validationState={this.getValidationState()}
						>
							<ControlLabel>{this.state.message}</ControlLabel>
							<FormControl
								type="text"
								value={this.state.value}
								placeholder={i18n.t("common.value")}
								onChange={this.handleChange}
							/>
							<FormControl.Feedback />
						</FormGroup>
					</form>
				</Modal.Body>

				<Modal.Footer>
					<Button bsStyle="primary" onClick={() => this.close(true)}>{i18n.t("common.ok")}</Button>
					<Button bsStyle="primary" onClick={() => this.close(false)}>{i18n.t("common.cancel")}</Button>
				</Modal.Footer>

			</Modal>
		);
	}
}

export default ModalEdit;
