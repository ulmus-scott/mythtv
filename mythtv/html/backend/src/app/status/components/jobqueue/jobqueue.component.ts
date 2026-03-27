import { Component, OnInit, Input } from '@angular/core';
import { JobQueueJob } from 'src/app/services/interfaces/jobqueue.interface';
import { UtilityService } from 'src/app/services/utility.service';
import { TranslateModule } from '@ngx-translate/core';
import { NgIf, NgFor } from '@angular/common';

@Component({
    selector: 'app-status-jobqueue',
    templateUrl: './jobqueue.component.html',
    styleUrls: ['./jobqueue.component.css', '../../status.component.css'],
    imports: [NgIf, NgFor, TranslateModule]
})
export class JobqueueComponent implements OnInit {
  @Input() jobqueue?: JobQueueJob[];

  constructor(public utility: UtilityService) { }

  ngOnInit(): void {
  }

}
