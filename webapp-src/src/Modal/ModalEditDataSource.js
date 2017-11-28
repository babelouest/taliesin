import React, { Component } from 'react';
import { Button, Modal, FormControl, FormGroup, Row, Col, Label } from 'react-bootstrap';
import StateStore from '../lib/StateStore';

class ModalEditDataSource extends Component {
  constructor(props) {
    super(props);
		
		this.state = {
			show: props.show, 
			dataSource: props.add?{name: "", description: "", icon: "", scope: "me", path: ""}:props.dataSource, 
			iconPath: "",
			onCloseCb: props.onCloseCb, 
			add: props.add
		};
		
		this.close = this.close.bind(this);
		this.handleChangeName = this.handleChangeName.bind(this);
		this.handleChangeDescription = this.handleChangeDescription.bind(this);
		this.handleChangeScope = this.handleChangeScope.bind(this);
		this.handleChangePath = this.handleChangePath.bind(this);
		this.handleUploadIcon = this.handleUploadIcon.bind(this);
		this.getNameValidationState = this.getNameValidationState.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({
			show: nextProps.show, 
			dataSource: nextProps.add?{name: "", description: "", icon: "", scope: "me", path: ""}:nextProps.dataSource, 
			iconPath: "",
			onCloseCb: nextProps.onCloseCb, 
			add: nextProps.add,
			nameValidateMessage: ""
		});
	}

  close(result) {
		if (result && this.state.onCloseCb) {
			this.state.onCloseCb(this.state.dataSource, this.state.add);
		} else {
			this.setState({ show: false }, () => {
				this.state.onCloseCb(false, this.state.add);
			});
		}
  }
	
	getNameValidationState() {
		if (this.state.add) {
			if (this.state.dataSource.name) {
				var found = StateStore.getState().dataSourceList.find((dataSource) => {
					return dataSource.name === this.state.dataSource.name;
				})
				if (found) {
					return "error";
				} else {
					return "success";
				}
			} else {
				return "error";
			}
		} else {
			return "success";
		}
	}
	
  handleChangeName(e) {
		var dataSource = this.state.dataSource;
		dataSource.name = e.target.value;
    this.setState({ dataSource: dataSource });
  }

  handleChangeDescription(e) {
		var dataSource = this.state.dataSource;
		dataSource.description = e.target.value;
    this.setState({ dataSource: dataSource });
  }
	
	handleChangeScope(e) {
		var dataSource = this.state.dataSource;
		dataSource.scope = e.target.value;
    this.setState({ dataSource: dataSource });
	}
	
	handleChangePath(e) {
		var dataSource = this.state.dataSource;
		dataSource.path = e.target.value;
    this.setState({ dataSource: dataSource });
	}
	
  render() {
		var title = this.state.add?"Add a new Data Source":"Edit Data Source '"+this.state.dataSource.name+"'", path = "";
		var scopeInput = <span>{this.state.dataSource.scope==="me"?"Me":"All"}</span>;
		if (StateStore.getState().profile.isAdmin && this.state.add) {
			scopeInput =
				<FormControl
					value={this.state.dataSource.scope}
					onChange={this.handleChangeScope}
					disabled={!this.state.add}
					componentClass="select"
					placeholder="select"
				>
					<option value={"me"}>Me</option>
					<option value={"all"}>All</option>
				</FormControl>;
		}
		if (this.state.add) {
			path = 
				<div>
					<Row>
						<Col md={12}>
							<hr/>
						</Col>
					</Row>
					<Row>
						<Col md={4}>
							<Label>Path on server</Label>
						</Col>
						<Col md={8}>
							<FormControl
								type="text"
								value={this.state.dataSource.path}
								placeholder="Enter a valid path"
								onChange={this.handleChangePath}
							/>
						</Col>
					</Row>
				</div>;
		}
		return (
			<Modal show={this.state.show}>
				<Modal.Header>
					<Modal.Title>{title}</Modal.Title>
				</Modal.Header>

				<Modal.Body>
					<form>
						<FormGroup
							controlId="formName"
							validationState={this.getNameValidationState()}
						>
							<Row>
								<Col md={4}>
									<Label>Name</Label>
								</Col>
								<Col md={8}>
									<FormControl
										type="text"
										value={this.state.dataSource.name}
										placeholder="Name"
										onChange={this.handleChangeName}
										disabled={!this.state.add}
									/>
								</Col>
							</Row>
						</FormGroup>
						<Row>
							<Col md={12}>
								<hr/>
							</Col>
						</Row>
						<Row>
							<Col md={4}>
								<Label>Description</Label>
							</Col>
							<Col md={8}>
								<FormControl
									type="text"
									value={this.state.dataSource.description}
									placeholder="Description"
									onChange={this.handleChangeDescription}
								/>
							</Col>
						</Row>
						<Row>
							<Col md={12}>
								<hr/>
							</Col>
						</Row>
						<Row>
							<Col md={4}>
								<Label>Scope</Label>
							</Col>
							<Col md={8}>
								{scopeInput}
							</Col>
						</Row>
						{path}
					</form>
				</Modal.Body>

				<Modal.Footer>
					<Button bsStyle="primary" onClick={() => this.close(true)} disabled={this.getNameValidationState() !== "success"}>OK</Button>
					<Button bsStyle="primary" onClick={() => this.close(false)}>Cancel</Button>
				</Modal.Footer>

			</Modal>
		);
	}
}

export default ModalEditDataSource;
