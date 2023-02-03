import React, { Component } from 'react';
import { Button, Modal, FormControl, FormGroup, ControlLabel, Checkbox } from 'react-bootstrap';

import i18n from '../lib/i18n';

class ModalResetStream extends Component {
	constructor(props) {
		super(props);
		
		this.state = {
      show: props.show,
      value: props.value,
      resetUrlRandom: true,
      cb: props.onCloseCb
    };
		
		this.close = this.close.bind(this);
		this.handleChange = this.handleChange.bind(this);
		this.handleChangeRandom = this.handleChangeRandom.bind(this);
		this.getValidationState = this.getValidationState.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({
      show: nextProps.show,
      value: nextProps.value,
      resetUrlRandom: true,
      cb: nextProps.onCloseCb
    });
	}

	close(result, e) {
		if (e) {
			e.preventDefault();
		}
		this.setState({ show: false }, () => {
			this.state.cb && this.state.cb(result, this.state.resetUrlRandom?false:this.state.value);
		});
	}
	
	getValidationState() {
		if (!this.state.value.length && !this.state.resetUrlRandom) {
			return "error";
		} else {
			return "success";
		}
	}

	handleChange(e) {
		this.setState({ value: e.target.value });
	}
  
  handleChangeRandom() {
		this.setState({ resetUrlRandom: !this.state.resetUrlRandom });
  }

	render() {
		return (
			<Modal show={this.state.show}>
				<Modal.Header>
					<Modal.Title>{i18n.t("stream.reset_url_title")}</Modal.Title>
				</Modal.Header>

				<Modal.Body>
					<form onSubmit={(e) => {this.close(true, e)}}>
						<FormGroup
							controlId="formResetUrl"
							validationState={this.getValidationState()}
						>
							<ControlLabel>{i18n.t("stream.reset_url_message")}</ControlLabel>
							<FormControl
								type="text"
								value={this.state.value}
								placeholder={i18n.t("stream.reset_url_value")}
								onChange={this.handleChange}
                disabled={this.state.resetUrlRandom}
                maxLength={32}
							/>
              <hr/>
              <ControlLabel>{i18n.t("stream.reset_url_random")}</ControlLabel>
              <Checkbox checked={this.state.resetUrlRandom} onChange={this.handleChangeRandom}>
              </Checkbox>
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

export default ModalResetStream;
