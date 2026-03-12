import { Component, OnInit, Input } from '@angular/core';
import { ScheduleOrProgram } from 'src/app/services/interfaces/program.interface';
import { UtilityService } from 'src/app/services/utility.service';
import { TranslateModule } from '@ngx-translate/core';
import { TooltipModule } from 'primeng/tooltip';
import { NgIf, NgFor } from '@angular/common';

@Component({
    selector: 'app-status-scheduled',
    templateUrl: './scheduled.component.html',
    styleUrls: ['./scheduled.component.css', '../../status.component.css'],
    standalone: true,
    imports: [NgIf, NgFor, TooltipModule, TranslateModule]
})
export class ScheduledComponent implements OnInit {
  @Input() scheduled?: ScheduleOrProgram[];

  constructor(public utility: UtilityService) { }

  ngOnInit(): void {
  }


}
