import React, { Component } from 'react';
import { Button, Modal, FormControl, FormGroup, Row, Col, Label, Image } from 'react-bootstrap';
import StateStore from '../lib/StateStore';

class ModalAddPlaylist extends Component {
  constructor(props) {
    super(props);
		
		this.state = {
			show: props.show, 
      add: props.add,
			playlist: props.add?{name: "", description: "", cover: undefined, scope: "me"}:props.playlist, 
			iconPath: "",
			onCloseCb: props.onCloseCb
		};
		
		this.close = this.close.bind(this);
		this.handleChangeName = this.handleChangeName.bind(this);
		this.handleChangeDescription = this.handleChangeDescription.bind(this);
		this.handleChangeScope = this.handleChangeScope.bind(this);
		this.handleUploadIcon = this.handleUploadIcon.bind(this);
		this.getNameValidationState = this.getNameValidationState.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({
			show: nextProps.show, 
      add: nextProps.add,
			playlist: nextProps.add?{name: "", description: "", cover: undefined, scope: "me"}:nextProps.playlist, 
			iconPath: "",
			onCloseCb: nextProps.onCloseCb,
			nameValidateMessage: ""
		});
	}

  close(result, e) {
		if (e) {
			e.preventDefault();
		}
		if (result && this.state.onCloseCb) {
			this.setState({ show: false }, () => {
        this.state.onCloseCb(true, this.state.playlist, this.state.add);
			});
		} else {
			this.setState({ show: false }, () => {
				this.state.onCloseCb(false, false, false);
			});
		}
  }
	
	getNameValidationState() {
    if (this.state.add) {
      if (this.state.playlist.name) {
        var found = StateStore.getState().playlist.find((playlist) => {
          return playlist.name === this.state.playlist.name;
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
		var playlist = this.state.playlist;
		playlist.name = e.target.value;
    this.setState({ playlist: playlist });
  }

  handleChangeDescription(e) {
		var playlist = this.state.playlist;
		playlist.description = e.target.value;
    this.setState({ playlist: playlist });
  }
	
	handleChangeScope(e) {
		var playlist = this.state.playlist;
		playlist.scope = e.target.value;
    this.setState({ playlist: playlist });
	}
	
	handleUploadIcon(e) {
		var file = e.target.files[0];
		var fr = new FileReader();
		var self = this;
		if (file.type.startsWith("image") && file.size <= 16*1024*1024) {
			fr.onload = function(ev2) {
				var encoded = btoa(ev2.target.result);
				if (encoded.length <= 16*1024*1024) {
					var playlist = self.state.playlist;
					playlist.cover = encoded;
					self.setState({playlist: playlist});
				} else {
					StateStore.getState().NotificationManager.addNotification({
						message: 'File too large, must be at least ~10MB',
						level: 'error'
					});
				}
			};
			fr.readAsBinaryString(file);
		} else {
			StateStore.getState().NotificationManager.addNotification({
				message: 'File error, only images allowed, must be at least ~10MB',
				level: 'error'
			});
		}
	}

  render() {
		var title = this.state.add?"Add a new Playlist":"Edit Playlist '"+this.state.playlist.name+"'";
    var scopeInput, cover;
		if (StateStore.getState().profile.isAdmin && this.state.add) {
			scopeInput =
				<FormControl
					value={this.state.playlist.scope}
					onChange={this.handleChangeScope}
					disabled={!this.state.add}
					componentClass="select"
					placeholder="select"
				>
					<option value={"me"}>Me</option>
					<option value={"all"}>All</option>
				</FormControl>;
		} else {
      if (!this.state.add) {
        scopeInput = <span>{this.state.playlist.scope==="me"?"Me":"All"}</span>;
      } else {
        scopeInput = <span>Me</span>;
      }
    }
    if (this.state.playlist.cover) {
      cover = 
        <div>
          <Row>
            <Col md={12}>
              <hr/>
            </Col>
          </Row>
          <Row>
            <Col md={4}>
              <Label>Icon selected</Label>
            </Col>
            <Col md={8}>
              <Image src={"data:image/jpeg;base64," + this.state.playlist.cover} responsive/>
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
					<form onSubmit={(e) => {this.close(true, e)}}>
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
										value={this.state.playlist.name}
                    disabled={!this.state.add}
										placeholder="Name"
										onChange={this.handleChangeName}
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
									value={this.state.playlist.description}
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
						<Row>
							<Col md={12}>
								<hr/>
							</Col>
						</Row>
						<Row>
							<Col md={4}>
								<Label>Icon</Label>
							</Col>
							<Col md={8}>
								<FormControl
									type="file"
									placeholder="Icon file"
									onChange={this.handleUploadIcon}
								/>
							</Col>
						</Row>
            {cover}
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

export default ModalAddPlaylist;
