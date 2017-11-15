import React, { Component } from 'react';
import { Button, Modal, FormControl, FormGroup, ControlLabel } from 'react-bootstrap';

class ModalEdit extends Component {
  constructor(props) {
    super(props);
		
		this.state = {show: props.show, title: props.title, message: props.message, value: props.value, cb: props.cb};
		
		this.close = this.close.bind(this);
		this.handleChange = this.handleChange.bind(this);
		this.getValidationState = this.getValidationState.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({show: nextProps.show, title: nextProps.title, message: nextProps.message, value: nextProps.value, cb: nextProps.cb});
	}

  close(result) {
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
					<form>
						<FormGroup
							controlId="formBasicText"
							validationState={this.getValidationState()}
						>
							<ControlLabel>{this.state.message}</ControlLabel>
							<FormControl
								type="text"
								value={this.state.value}
								placeholder="Value"
								onChange={this.handleChange}
							/>
							<FormControl.Feedback />
						</FormGroup>
					</form>
				</Modal.Body>

				<Modal.Footer>
					<Button bsStyle="primary" onClick={() => this.close(true)}>OK</Button>
					<Button bsStyle="primary" onClick={() => this.close(false)}>Cancel</Button>
				</Modal.Footer>

			</Modal>
		);
	}
}

export default ModalEdit;
