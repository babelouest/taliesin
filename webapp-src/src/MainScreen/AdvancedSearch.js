import React, { Component } from 'react';
import { Row, Col, Button, FormGroup, ControlLabel, FormControl, Checkbox, DropdownButton, MenuItem, InputGroup, Image, Panel } from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';
import Datetime from 'react-datetime';
import 'react-datetime/css/react-datetime.css';
import StateStore from '../lib/StateStore';
import ModalMedia from '../Modal/ModalMedia';

class AdvancedSearch extends Component {
  constructor(props) {
    super(props);
		
		this.state = {
			criteria: {
				query: "",
				type: "",
				data_source: StateStore.getState().dataSourceList.map((ds) => {return ds.name;}),
				tags: [],
				metrics: {},
				sort: "path",
				sort_direction: "asc",
				limit: 100,
				offset: 0
			},
      addMetrics: false,
			searchResult: [],
			searching: false,
			searchPanel: true,
			firstLoad: true,
			modalShow: false,
			modalMedia: false,
			modalTitle: ""
		};
		
		this.handleSelectPanel = this.handleSelectPanel.bind(this);
		this.changeSelectDataSource = this.changeSelectDataSource.bind(this);
		this.handleChangeQuery = this.handleChangeQuery.bind(this);
		this.handleChangeType = this.handleChangeType.bind(this);
		this.handleAddTag = this.handleAddTag.bind(this);
		this.handleChangeTagKey = this.handleChangeTagKey.bind(this);
		this.handleSetTagKey = this.handleSetTagKey.bind(this);
		this.handleSetTagOperator = this.handleSetTagOperator.bind(this);
		this.handleChangeTagValue = this.handleChangeTagValue.bind(this);
		this.handleAddMetrics = this.handleAddMetrics.bind(this);
		this.handleSetMetrics = this.handleSetMetrics.bind(this);
		this.handleSetMetricsOperator = this.handleSetMetricsOperator.bind(this);
		this.handleChangeMetricsValue = this.handleChangeMetricsValue.bind(this);
		this.handleChangeMetricsValueMax = this.handleChangeMetricsValueMax.bind(this);
		this.handleRemoveMetrics = this.handleRemoveMetrics.bind(this);
		this.handleChangeMetricsPlayedAt = this.handleChangeMetricsPlayedAt.bind(this);
		this.handleChangeMetricsPlayedAtMax = this.handleChangeMetricsPlayedAtMax.bind(this);
		this.handleChangeMetricsLastSeen = this.handleChangeMetricsLastSeen.bind(this);
		this.handleChangeMetricsLastSeenMax = this.handleChangeMetricsLastSeenMax.bind(this);
		this.handleChangeMetricsLastUpdated = this.handleChangeMetricsLastUpdated.bind(this);
		this.handleChangeMetricsLastUpdatedMax = this.handleChangeMetricsLastUpdatedMax.bind(this);
		this.runSearch = this.runSearch.bind(this);
		this.openMedia = this.openMedia.bind(this);
		this.closeMedia = this.closeMedia.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
    this.setState({
			criteria: {
				query: "",
				type: "",
				data_source: StateStore.getState().dataSourceList.map((ds) => {return ds.name;}),
				tags: [],
				metrics: {},
				sort: "path",
				sort_direction: "asc",
				limit: 100,
				offset: 0
			},
      addMetrics: false,
			searchResult: [],
			searching: false,
			searchPanel: true,
			firstLoad: true,
			modalShow: false,
			modalMedia: false,
			modalTitle: ""
		});
	}
	
  getValidationState() {
		return "success";
  }
	
	handleSelectPanel() {
		this.setState({searchPanel: !this.state.searchPanel})
	}
	
	handleChangeType(e) {
		var criteria = this.state.criteria;
		criteria.type = e.target.value;
		this.setState({criteria: criteria});
	}
	
	changeSelectDataSource(dataSource) {
		var criteria = this.state.criteria;
		var index = criteria.data_source.indexOf(dataSource);
		
		if (index === -1) {
			criteria.data_source.push(dataSource);
		} else {
			criteria.data_source.splice(index, 1);
		}
		this.setState({criteria: criteria});
	}
  
  handleChangeQuery(e) {
		var criteria = this.state.criteria;
    if (e.target.value !== "") {
      criteria.query = e.target.value;
    } else {
      delete criteria.query;
    }
    this.setState({criteria: criteria});
  }
	
	handleAddTag() {
		var criteria = this.state.criteria;
		criteria.tags.push({key: "", operator: "contains", value: ""});
		this.setState({criteria: criteria});
	}
	
	handleChangeTagKey(index, e) {
		var criteria = this.state.criteria;
		criteria.tags[index].key = e.target.value;
		this.setState({criteria: criteria});
	}
	
	handleSetTagKey(index, key) {
		var criteria = this.state.criteria;
		criteria.tags[index].key = key;
		this.setState({criteria: criteria});
	}
	
	handleSetTagOperator(index, e) {
		var criteria = this.state.criteria;
		criteria.tags[index].operator = e.target.value;
		this.setState({criteria: criteria});
	}
	
	handleChangeTagValue(index, e) {
		var criteria = this.state.criteria;
		criteria.tags[index].value = e.target.value;
		this.setState({criteria: criteria});
	}
  
  handleAddMetrics() {
    var metricsList = [<option key="0" value="">Select new metrics</option>], addMetrics;
    if (!this.state.criteria.metrics.nb_play) {
      metricsList.push(
        <option key="1" value="nb_play">Number of times played</option>
      );
    }
    if (!this.state.criteria.metrics.played_at) {
      metricsList.push(
        <option key="2" value="played_at">Date played</option>
      );
    }
    if (!this.state.criteria.metrics.last_seen) {
      metricsList.push(
        <option key="3" value="last_seen">Date last seen</option>
      );
    }
    if (!this.state.criteria.metrics.last_updated) {
      metricsList.push(
        <option key="4" value="last_updated">Date last updated</option>
      );
    }
    addMetrics = 
    <FormControl componentClass="select" placeholder="select" onChange={(e) => this.handleSetMetrics(e)}>
    {metricsList}
    </FormControl>;
    this.setState({addMetrics: addMetrics});
  }
  
  handleSetMetrics(e) {
    var criteria = this.state.criteria;
    switch (e.target.value) {
      case "nb_play":
        criteria.metrics.nb_play = {operator: "equals", value: 0, value_max: 0};
        break;
      case "played_at":
        criteria.metrics.played_at = {operator: "equals", value: 0, value_max: 0};
        break;
      case "last_seen":
        criteria.metrics.last_seen = {operator: "equals", value: 0, value_max: 0};
        break;
      case "last_updated":
        criteria.metrics.last_updated = {operator: "equals", value: 0, value_max: 0};
        break;
      default:
        break;
    }
    this.setState({criteria: criteria, addMetrics: false});
  }
  
  handleSetMetricsOperator(metrics, e) {
    var criteria = this.state.criteria;
    criteria.metrics[metrics].operator = e.target.value;
    this.setState({criteria: criteria, addMetrics: false});
  }
  
  handleChangeMetricsValue(metrics, e) {
    var criteria = this.state.criteria;
    criteria.metrics[metrics].value = parseInt(e.target.value, 10);
    this.setState({criteria: criteria, addMetrics: false});
  }
  
  handleChangeMetricsValueMax(metrics, e) {
    var criteria = this.state.criteria;
    criteria.metrics[metrics].value_max = parseInt(e.target.value, 10);
    this.setState({criteria: criteria, addMetrics: false});
  }
  
  handleChangeMetricsPlayedAt(moment) {
    var criteria = this.state.criteria;
    criteria.metrics.played_at.value = Math.round(moment.valueOf() / 1000);
    this.setState({criteria: criteria, addMetrics: false});
  }
  
  handleChangeMetricsPlayedAtMax(moment) {
    var criteria = this.state.criteria;
    criteria.metrics.played_at.value_max = Math.round(moment.valueOf() / 1000);
    this.setState({criteria: criteria, addMetrics: false});
  }
  
  handleChangeMetricsLastSeen(moment) {
    var criteria = this.state.criteria;
    criteria.metrics.last_seen.value = Math.round(moment.valueOf() / 1000);
    this.setState({criteria: criteria, addMetrics: false});
  }
  
  handleChangeMetricsLastSeenMax(moment) {
    var criteria = this.state.criteria;
    criteria.metrics.last_seen.value_max = Math.round(moment.valueOf() / 1000);
    this.setState({criteria: criteria, addMetrics: false});
  }
  
  handleChangeMetricsLastUpdated(moment) {
    var criteria = this.state.criteria;
    criteria.metrics.last_updated.value = Math.round(moment.valueOf() / 1000);
    this.setState({criteria: criteria, addMetrics: false});
  }
  
  handleChangeMetricsLastUpdatedMax(moment) {
    var criteria = this.state.criteria;
    criteria.metrics.last_updated.value_max = Math.round(moment.valueOf() / 1000);
    this.setState({criteria: criteria, addMetrics: false});
  }
  
  handleRemoveMetrics(metrics) {
    var criteria = this.state.criteria;
    delete criteria.metrics[metrics];
    this.setState({criteria: criteria, addMetrics: false});
  }
  
  runSearch(e) {
		if (e) {
			e.preventDefault();
		}
		this.setState({searching: true, searchResult: [], firstLoad: false, searchPanel: false}, () => {
			StateStore.getState().APIManager.taliesinApiRequest("PUT", "/search/", this.state.criteria)
			.then((result) => {
				this.setState({searchResult: result, searching: false}, () => {
					var searchResult = this.state.searchResult;
					searchResult.forEach((media) => {
						if (!media.cover) {
							StateStore.getState().APIManager.taliesinApiRequest("GET", "/data_source/" + encodeURIComponent(media.data_source) + "/browse/path/" + encodeURI(media.path).replace(/#/g, "%23").replace(/\+/g, "%2B") + "?cover&thumbnail&base64")
							.then((cover) => {
								media.cover = cover;
								this.setState({searchResult: searchResult});
							});
						}
					});
				});
			})
			.fail(() => {
				this.setState({searchResult: [], searching: false});
			});
		});
  }
	
	openMedia(media) {
		this.setState({modalShow: true, modalMedia: media, modalTitle: (media.tags.title||media.name)});
	}

	closeMedia() {
		this.setState({modalShow: false});
	}

	render() {
		var dataSourceList = [], tagList = [], metricsList = [], addMetrics, resultList = [], resultTable, resultEmpty;
		StateStore.getState().dataSourceList.forEach((dataSource, index) => {
			dataSourceList.push(
				<Checkbox key={index} inline checked={this.state.criteria.data_source.indexOf(dataSource.name)!==-1} onChange={() => this.changeSelectDataSource(dataSource.name)}>
					{dataSource.name}
				</Checkbox>
			);
		});
		this.state.criteria.tags.forEach((tag, index) => {
			var keyName = "";
			if (tag.key) {
				keyName = tag.key;
			}
			tagList.push(
				<Row key={index}>
					<Col xs={2}>
						<ControlLabel>Key</ControlLabel>
						<InputGroup>
							<DropdownButton title={keyName||"Select key"} id="keyList" className="form-control">
								<MenuItem onClick={() => this.handleSetTagKey(index, "title")}>Title</MenuItem>
								<MenuItem onClick={() => this.handleSetTagKey(index, "artist")}>Artist</MenuItem>
								<MenuItem onClick={() => this.handleSetTagKey(index, "album")}>Album</MenuItem>
								<MenuItem onClick={() => this.handleSetTagKey(index, "date")}>Year</MenuItem>
								<MenuItem onClick={() => this.handleSetTagKey(index, "genre")}>Genre</MenuItem>
								<MenuItem divider />
								<MenuItem>
									<FormControl
										type="text"
										value={keyName}
										placeholder="Tag key"
										autoFocus
										onChange={(e) => {this.handleChangeTagKey(index, e)}}
									/>
								</MenuItem>
							</DropdownButton>
						</InputGroup>
					</Col>
					<Col xs={4}>
						<ControlLabel>Operator</ControlLabel>
            <FormControl componentClass="select" placeholder="select" value={tag.operator} onChange={(e) => this.handleSetTagOperator(index, e)}>
              <option value="contains">Contains</option>
              <option value="equals">Equals</option>
              <option value="different">Different</option>
              <option value="lower">Lower</option>
              <option value="higher">Higher</option>
              <option value="empty">Empty</option>
            </FormControl>
					</Col>
					<Col xs={4}>
						<ControlLabel>Value</ControlLabel>
						<FormControl
							type="text"
							value={tag.name}
							placeholder="Tag name"
							onChange={(e) => {this.handleChangeTagValue(index, e)}}
							disabled={tag.operator==="empty"}
						/>
					</Col>
					<Col xs={2}>
						<ControlLabel>Remove</ControlLabel>
						<InputGroup>
							<Button title="Remove" onClick={() => this.handleRemoveTag(index)}>
								<FontAwesome name="times"/>
							</Button>
						</InputGroup>
					</Col>
				</Row>
			);
		});
    if (this.state.addMetrics) {
      addMetrics = 
      <Row>
        <Col>
          <ControlLabel>New metrics value</ControlLabel>
          {this.state.addMetrics}
        </Col>
      </Row>;
    }
    var valueMax;
    if (this.state.criteria.metrics.nb_play) {
      valueMax = "";
      if (this.state.criteria.metrics.nb_play.operator === "between") {
        valueMax =
        <div>
          <ControlLabel>Value max</ControlLabel>
          <FormControl
            type="text"
            value={this.state.criteria.metrics.nb_play.value_max}
            placeholder="Value"
            onChange={(e) => {this.handleChangeMetricsValueMax("nb_play", e)}}
          />
        </div>
      }
      metricsList.push(
        <Row key={0}>
          <Col xs={2}>
            <span className="label label-info">
              Number of times played
            </span>
          </Col>
          <Col xs={4}>
						<ControlLabel>Operator</ControlLabel>
            <FormControl componentClass="select" placeholder="select" value={this.state.criteria.metrics.nb_play.operator} onChange={(e) => this.handleSetMetricsOperator("nb_play", e)}>
              <option value="equals">Equals</option>
              <option value="different">Different</option>
              <option value="lower">Lower</option>
              <option value="higher">Higher</option>
              <option value="between">Between</option>
            </FormControl>
          </Col>
          <Col xs={4}>
						<ControlLabel>Value</ControlLabel>
						<FormControl
							type="text"
							value={this.state.criteria.metrics.nb_play.value}
							placeholder="Value"
							onChange={(e) => {this.handleChangeMetricsValue("nb_play", e)}}
						/>
            {valueMax}
          </Col>
          <Col xs={2}>
						<ControlLabel>Remove</ControlLabel>
						<InputGroup>
							<Button title="Remove" onClick={() => this.handleRemoveMetrics("nb_play")}>
								<FontAwesome name="times"/>
							</Button>
						</InputGroup>
          </Col>
        </Row>
      );
    }
    if (this.state.criteria.metrics.played_at) {
      valueMax = "";
      if (this.state.criteria.metrics.played_at.operator === "between") {
        valueMax =
        <div>
          <ControlLabel>Value max</ControlLabel>
          <Datetime value={this.state.criteria.metrics.played_at.value_max*1000} onChange={this.handleChangeMetricsPlayedAtMax}/>
        </div>
      }
      metricsList.push(
        <Row key={1}>
          <Col xs={2}>
            <span className="label label-info">
              Played at
            </span>
          </Col>
          <Col xs={4}>
						<ControlLabel>Operator</ControlLabel>
            <FormControl componentClass="select" placeholder="select" value={this.state.criteria.metrics.played_at.operator} onChange={(e) => this.handleSetMetricsOperator("played_at", e)}>
              <option value="equals">On</option>
              <option value="different">Not on</option>
              <option value="lower">Before</option>
              <option value="higher">After</option>
              <option value="between">Between</option>
            </FormControl>
          </Col>
          <Col xs={4}>
						<ControlLabel>Value</ControlLabel>
            <Datetime value={this.state.criteria.metrics.played_at.value*1000} onChange={this.handleChangeMetricsPlayedAt}/>
            {valueMax}
          </Col>
          <Col xs={2}>
						<ControlLabel>Remove</ControlLabel>
						<InputGroup>
							<Button title="Remove" onClick={() => this.handleRemoveMetrics("played_at")}>
								<FontAwesome name="times"/>
							</Button>
						</InputGroup>
          </Col>
        </Row>
      );
    }
    if (this.state.criteria.metrics.last_seen) {
      valueMax = "";
      if (this.state.criteria.metrics.last_seen.operator === "between") {
        valueMax =
        <div>
          <ControlLabel>Value max</ControlLabel>
          <Datetime value={this.state.criteria.metrics.last_seen.value_max*1000} onChange={this.handleChangeMetricsLastSeenMax}/>
        </div>
      }
      metricsList.push(
        <Row key={2}>
          <Col xs={2}>
            <span className="label label-info">
              Last seen
            </span>
          </Col>
          <Col xs={4}>
						<ControlLabel>Operator</ControlLabel>
            <FormControl componentClass="select" placeholder="select" value={this.state.criteria.metrics.last_seen.operator} onChange={(e) => this.handleSetMetricsOperator("last_seen", e)}>
              <option value="equals">On</option>
              <option value="different">Not on</option>
              <option value="lower">Before</option>
              <option value="higher">After</option>
              <option value="between">Between</option>
            </FormControl>
          </Col>
          <Col xs={4}>
						<ControlLabel>Value</ControlLabel>
						<Datetime value={this.state.criteria.metrics.last_seen.value*1000} onChange={this.handleChangeMetricsLastSeen}/>
            {valueMax}
          </Col>
          <Col xs={2}>
						<ControlLabel>Remove</ControlLabel>
						<InputGroup>
							<Button title="Remove" onClick={() => this.handleRemoveMetrics("last_seen")}>
								<FontAwesome name="times"/>
							</Button>
						</InputGroup>
          </Col>
        </Row>
      );
    }
    if (this.state.criteria.metrics.last_updated) {
      valueMax = "";
      if (this.state.criteria.metrics.last_updated.operator === "between") {
        valueMax =
        <div>
          <ControlLabel>Value max</ControlLabel>
          <Datetime value={this.state.criteria.metrics.last_updated.value_max*1000} onChange={this.handleChangeMetricsLastUpdatedMax}/>
        </div>
      }
      metricsList.push(
        <Row key={3}>
          <Col xs={2}>
            <span className="label label-info">
              Last Updated
            </span>
          </Col>
          <Col xs={4}>
						<ControlLabel>Operator</ControlLabel>
            <FormControl componentClass="select" placeholder="select" value={this.state.criteria.metrics.last_updated.operator} onChange={(e) => this.handleSetMetricsOperator("last_updated", e)}>
              <option value="equals">On</option>
              <option value="different">Not on</option>
              <option value="lower">Before</option>
              <option value="higher">After</option>
              <option value="between">Between</option>
            </FormControl>
          </Col>
          <Col xs={4}>
						<ControlLabel>Value</ControlLabel>
						<Datetime value={this.state.criteria.metrics.last_updated.value*1000} onChange={this.handleChangeMetricsLastUpdated}/>
            {valueMax}
          </Col>
          <Col xs={2}>
						<ControlLabel>Remove</ControlLabel>
						<InputGroup>
							<Button title="Remove" onClick={() => this.handleRemoveMetrics("last_updated")}>
								<FontAwesome name="times"/>
							</Button>
						</InputGroup>
          </Col>
        </Row>
      );
    }
		this.state.searchResult.forEach((result, index) => {
			var cover;
			if (result.cover) {
				cover = <Image src={"data:image/jpeg;base64," + result.cover} responsive style={{maxWidth: "100px", maxHeight: "100px"}}/>
			}
			resultList.push(
				<tr key={index}>
					<td>
						<a role="button" onClick={() => {this.openMedia(result)}}>
							{result.data_source}
						</a>
					</td>
					<td>
						<a role="button" onClick={() => {this.openMedia(result)}}>
							{result.tags.title?result.tags.title:result.name}
						</a>
					</td>
					<td>
						<a role="button" onClick={() => {this.openMedia(result)}}>
							{result.path}
						</a>
					</td>
					<td>
						<a role="button" onClick={() => {this.openMedia(result)}}>
							{cover}
						</a>
					</td>
				</tr>
			);
		});
		resultEmpty =
			<tr>
				<td colspan="4">
					<strong>No result found</strong>
				</td>
			</tr>;
		if (!this.state.firstLoad) {
			if (!this.state.searching) {
				resultTable =
					<table className="table table-striped table-hover">
						<thead>
							<tr>
								<th>
									Data Source
								</th>
								<th>
									Name
								</th>
								<th>
									Path
								</th>
								<th>
									Cover
								</th>
							</tr>
						</thead>
						<tbody>
							{resultList.length?resultList:resultEmpty}
						</tbody>
					</table>;
			} else {
				resultTable =
					<div>
						Searching... <FontAwesome name="spinner" spin />
					</div>
			}
		}
		return (
			<div>
				<form onSubmit={(e) => {this.runSearch(e)}}>
					<FormGroup
						controlId="formBasicText"
						validationState={this.getValidationState()}
					>
						<Panel collapsible header="Search query" onSelect={this.handleSelectPanel} expanded={this.state.searchPanel}>
							<Row>
								<ControlLabel>Search query</ControlLabel>
								<FormControl
									type="text"
									value={this.state.criteria.query}
									placeholder="Text to search"
									onChange={this.handleChangeQuery}
								/>
							</Row>
							<Row>
								<ControlLabel>Media type</ControlLabel>
								<FormControl componentClass="select" placeholder="select" value={this.state.criteria.type} onChange={this.handleChangeType}>
									<option value="">All</option>
									<option value="audio">Audio</option>
									<option value="video">Video</option>
									<option value="image">Image</option>
									<option value="subtitle">Subtitle</option>
									<option value="unknown">Unknown</option>
								</FormControl>
							</Row>
							<Row>
								<hr/>
							</Row>
							<Row>
								<h3>Data source</h3>
							</Row>
							<Row>
								{dataSourceList}
							</Row>
							<Row>
								<hr/>
							</Row>
							<Row>
								<h3>Tags</h3>
							</Row>
							<Row>
								{tagList}
							</Row>
							<Row style={{paddingTop: "10px"}}>
								<Button title="Add tag" onClick={this.handleAddTag}>
									<FontAwesome name="plus" />
								</Button>
							</Row>
							<Row>
								<hr/>
							</Row>
							<Row>
								<h3>Metrics</h3>
							</Row>
							{metricsList}
							{addMetrics}
							<Row style={{paddingTop: "10px"}}>
								<Button title="Add tag" onClick={this.handleAddMetrics}>
									<FontAwesome name="plus" />
								</Button>
							</Row>
							<Row>
								<hr/>
							</Row>
						</Panel>
						<Row>
							<Button title="Search" onClick={this.runSearch}>
								Search&nbsp;<FontAwesome name="search" />
							</Button>
						</Row>
					</FormGroup>
				</form>
				{resultTable}
				<ModalMedia show={this.state.modalShow} media={this.state.modalMedia} title={this.state.modalTitle} onClose={this.closeMedia} />
			</div>
		);
	}
}

export default AdvancedSearch;
