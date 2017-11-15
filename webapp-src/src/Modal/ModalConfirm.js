import React, { Component } from 'react';
import { Button, Modal } from 'react-bootstrap';

class ModalConfirm extends Component {
  constructor(props) {
    super(props);
		
		this.state = {show: props.show, title: props.title, message: props.message, cb: props.cb};
		
		this.close = this.close.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({show: nextProps.show, title: nextProps.title, message: nextProps.message, cb: nextProps.cb});
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
					<Button bsStyle="primary" onClick={() => this.close(true)}>OK</Button>
					<Button bsStyle="primary" onClick={() => this.close(false)}>Cancel</Button>
				</Modal.Footer>

			</Modal>
		);
	}
}

export default ModalConfirm;
