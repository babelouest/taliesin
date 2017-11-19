import React, { Component } from 'react';
import { Button, Modal, Row, Col, Label, FormControl } from 'react-bootstrap';
import StateStore from '../lib/StateStore';

class ModalEditStream extends Component {
  constructor(props) {
    super(props);
		
		this.state = {
			show: props.show, 
			dataSource: props.dataSource, 
			path: props.path,
			element: props.element,
			onCloseCb: props.onCloseCb,
			recursive: true
		};
		
		console.log(StateStore.getState().serverConfig)
		
		this.close = this.close.bind(this);
		this.handleChangeRecursive = this.handleChangeRecursive.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({
			show: nextProps.show, 
			dataSource: nextProps.dataSource, 
			path: nextProps.path,
			element: nextProps.element,
			onCloseCb: nextProps.onCloseCb,
			recursive: true
		});
	}

  close(result) {
    this.state.onCloseCb();
  }
	
	handleChangeRecursive() {
		
	}
	
  render() {
		var recursive;
		if (this.state.element.type === "folder") {
			recursive = 
			<div>
				<Row>
					<Col md={4}>
						<Label>Recursive</Label>
					</Col>
					<Col md={8}>
						<FormControl
							type="checkbox"
							value={this.state.recursive}
							placeholder="Enter a valid path"
							onChange={this.handleChangeRecursive}
						/>
					</Col>
				</Row>
				<Row>
					<Col md={12}>
						<hr/>
					</Col>
				</Row>
			</div>
		}
		return (
			<Modal show={this.state.show}>
				<Modal.Header>
					<Modal.Title>Create Stream</Modal.Title>
				</Modal.Header>
				<Modal.Body>
					<form>
						<Row>
							<Col md={4}>
								<Label>Path</Label>
							</Col>
							<Col md={8}>
                {this.state.dataSource + " - " + this.state.path}
							</Col>
						</Row>
						<Row>
							<Col md={12}>
								<hr/>
							</Col>
						</Row>
						{recursive}
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

export default ModalEditStream;
