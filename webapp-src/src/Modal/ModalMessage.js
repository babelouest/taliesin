import React, { Component } from 'react';
import { Button, Modal } from 'react-bootstrap';

class ModalMessage extends Component {
  constructor(props) {
    super(props);
		
		this.state = {show: props.show, title: props.title, message: props.message};
		
		this.close = this.close.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({show: nextProps.show, title: nextProps.title, message: nextProps.message});
	}

  close() {
    this.setState({ show: false });
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
					<Button bsStyle="primary" onClick={this.close}>Close</Button>
				</Modal.Footer>

			</Modal>
		);
	}
}

export default ModalMessage;
